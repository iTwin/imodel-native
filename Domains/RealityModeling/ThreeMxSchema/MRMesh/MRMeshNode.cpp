/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshNode::~MRMeshNode() 
    { 
    MRMeshCacheManager::GetManager().RemoveRequest(*this); 
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::Clear()
    {
    m_dir.clear();
    m_children.clear();
    m_meshes.clear();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::PushNode(S3NodeInfo const& nodeInfo)
    {
    if (nodeInfo.m_children.empty())
        return;

    m_children.push_back(new MRMeshNode(nodeInfo, this, m_system));
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::AddGeometry(int32_t nodeId, int32_t nbVertices, FPoint3d const* positions, FPoint3d const* normals, int32_t nbTriangles, uint32_t const* indices, FPoint2d const* textureCoordinates, int32_t textureIndex, SystemP system)
    {
    ByteStream* jpeg = GetTextureData(textureIndex);
    if (nullptr == jpeg) 
        {
        BeAssert(false); // ???
        return;
        }

    RgbImageInfo imageInfo;
    Render::Image rgba;
    if (SUCCESS != imageInfo.ReadImageFromJpgBuffer(rgba, jpeg->GetData(), jpeg->GetSize()))
        return;

    TexturePtr texture;
    if (system)
        {
        texture = system->_CreateTexture(rgba, imageInfo.m_hasAlpha);
        jpeg->Clear(); // we no longer need the jpeg data
        }
    
    m_meshes.push_back(new MRMeshGeometry(nbVertices, positions, normals, nbTriangles, indices, textureCoordinates, texture.get(), system));
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName MRMeshNode::GetFileName() const 
    {
    return m_info.m_children.empty() ? BeFileName() : MRMeshUtil::ConstructNodeName(m_info.m_children.front(), nullptr == m_parent ? nullptr : &m_parent->m_dir); 
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::Load()
    {
    if (!m_dir.empty())
        return SUCCESS;         // Already loaded.

    if (1 != m_info.m_children.size())
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != MRMeshCacheManager::GetManager().SynchronousRead(*this, GetFileName(), m_system))
        {
        MRMeshUtil::DisplayNodeFailureWarning(GetFileName().c_str());
        m_dir.clear();
        return ERROR;
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::LoadUntilDisplayable()
    {
    static uint32_t s_sleepMillis = 20;

    while (!LoadedUntilDisplayable())
        {
        MRMeshCacheManager::GetManager().ProcessRequests();
        RequestLoadUntilDisplayable();
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
bool MRMeshNode::LoadedUntilDisplayable() const
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
void MRMeshNode::RequestLoadUntilDisplayable()
    {
    if (IsLoaded() && !IsDisplayable())
        {
        m_primary = true;           // Mark this as a primary node (will never get flushed).

        if (!m_childrenRequested)
            {
            m_childrenRequested = true;
            MRMeshCacheManager::GetManager().QueueChildLoad(m_children, nullptr, Transform::FromIdentity(), m_system);
            }
        else
            {
            for (auto const& child : m_children)
                child->RequestLoadUntilDisplayable();
            }
        }
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MRMeshNode::AreChildrenLoaded() const
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
bool MRMeshNode::Validate(MRMeshNodeCP parent) const
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
bool MRMeshNode::IsCached() const
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
bool MRMeshNode::AreVisibleChildrenLoaded(MeshNodes& unloadedVisibleChildren, ViewContextR viewContext, MRMeshContextCR meshContext) const
    {
    if (meshContext.GetLoadSynchronous())
        {
        // If the LOD filter is off we assume that this is an application that is interested in full detail (and isn't going to wait for nodes to load.
        // We synchronously load unloaded but visible nodes and release non-visible nodes.
        for (auto& child : m_children)
            {
            if (child->IsDisplayable())
                {
                if (viewContext.IsPointVisible(child->m_info.m_center, ViewContext::WantBoresite::No, child->m_info.m_radius))
                    child->Load();
                else if (!child->m_primary)
                    child->Clear();
                }
            }
        return true;
        }

    for (auto const& child : m_children)
        {
        if (!child->IsLoaded() && child->IsDisplayable() && viewContext.IsPointVisible(child->m_info.m_center, ViewContext::WantBoresite::No, child->m_info.m_radius))
            unloadedVisibleChildren.push_back(child);
        }

    return unloadedVisibleChildren.empty();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawMeshes(RenderContextR context, MRMeshContextCR meshContext)
    {
    if (!IsLoaded())
        {
        BeAssert(false);
        return;
        }

    for (auto const& mesh : m_meshes)
        mesh->Draw(context, *this, meshContext);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawBoundingSphere(RenderContextR context) const
    {
    Render::GraphicPtr graphic = context.CreateGraphic(Render::Graphic::CreateParams());

    graphic->AddSolidPrimitive(*ISolidPrimitive::CreateDgnSphere(DgnSphereDetail(m_info.m_center, m_info.m_radius)));
    graphic->Close();
    context.OutputGraphic(*graphic, nullptr);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d MRMeshNode::GetRange() const
    {
    DRange3d range;
    range.InitFrom(m_info.m_center);
    range.Extend(m_info.m_radius);
    return range;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawMeshes(Render::GraphicR graphic, TransformCR transform)
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
bool MRMeshNode::TestVisibility(bool& isUnderMaximumSize, ViewContextR viewContext, MRMeshContextCR meshContext)
    {
    if (!viewContext.IsPointVisible(m_info.m_center, ViewContext::WantBoresite::No, m_info.m_radius))
        return false;

    if (meshContext.UseFixedResolution())
        {
        isUnderMaximumSize = (m_info.m_radius / meshContext.GetFixedResolution()) < m_info.m_dMax;
        }
    else
        {
        double pixelSize  =  m_info.m_radius / viewContext.GetPixelSizeAtPoint(&m_info.m_center);

        isUnderMaximumSize = pixelSize < MRMeshUtil::CalculateResolutionRatio() * m_info.m_dMax;
        }
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::Draw(bool& childrenScheduled, RenderContextR context, MRMeshContextCR meshContext)
    {
    bool isUnderMaximumSize = false, childrenNeedLoading = false;

    if (!TestVisibility(isUnderMaximumSize, context, meshContext) || !IsLoaded())
        return SUCCESS;

    m_lastUsed = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    MeshNodes unloadedVisibleChildren;
    
    if ((!isUnderMaximumSize || !IsDisplayable()) && 
        ! m_children.empty() && 
        ! (childrenNeedLoading = !AreVisibleChildrenLoaded(unloadedVisibleChildren, context, meshContext)))
        {                                                                                                 
        for (auto const& child : m_children)
            child->Draw(childrenScheduled, context, meshContext);

        return SUCCESS;
        }

    DrawMeshes(context, meshContext);

    if (meshContext.GetLoadSynchronous() &&         // If we just are exporting fixed resolution clear here to mimimize memory usage.
        meshContext.UseFixedResolution() &&
        !m_primary)
        Clear();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if ((DrawPurpose::Update  == viewContext.GetDrawPurpose() || DrawPurpose::UpdateHealing == viewContext.GetDrawPurpose()) &&
        !isUnderMaximumSize &&
        childrenNeedLoading &&
        nullptr != viewContext.GetViewport())
        {
        childrenScheduled = true;
        MRMeshCacheManager::GetManager().QueueChildLoad(unloadedVisibleChildren, viewContext.GetViewport(), meshContext.GetTransform());
        }
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshNode::GetMeshCount() const
    {
    size_t count = m_meshes.size();

    for (auto const& child : m_children)
        count += child->GetMeshCount();

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshNode::GetMeshMemorySize() const
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
size_t MRMeshNode::GetTextureMemorySize() const
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
size_t MRMeshNode::GetNodeCount() const
    {
    size_t  count = IsLoaded() ? 1 : 0;

    for (auto const& child : m_children)
        count += child->GetNodeCount();

    return count;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MRMeshNode::GetMaxDepth() const
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
void MRMeshNode::RemoveChild(MRMeshNodeP failedChild)
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
void MRMeshNode::ClearGraphics()
    {
    for (auto const& mesh : m_meshes)
        mesh->ClearGraphic();

    for (auto const& child : m_children)
        child->ClearGraphics();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::Clone(MRMeshNode const& other)
    {
    BeAssert(&other != this);
    m_system    = other.m_system;
    m_parent    = other.m_parent;
    m_info      = other.m_info;
    m_dir       = other.m_dir;
    m_meshes    = other.m_meshes;
    m_children.clear();

    for (auto const& otherChild : other.m_children)
        {
        MRMeshNodePtr child = new MRMeshNode(S3NodeInfo(), this, m_system);
        child->Clone(*otherChild);
        child->m_parent = this;
        m_children.push_back(child);
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  MRMeshNode::GetDepthMap(bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth)
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


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::GetRange(DRange3dR range, TransformCR transform) const
    {
    range = DRange3d::NullRange();

    for (auto const& child : m_children)     // Prefer the more accurate child range...
        {
        DRange3d        childRange = DRange3d::NullRange();

        if (SUCCESS == child->GetRange(childRange, transform))
            range.UnionOf(range, childRange);
        }

    if (range.IsNull())
        {
        for (auto const& mesh : m_meshes)
            {
            DRange3d        meshRange = DRange3d::NullRange();

            if (SUCCESS == mesh->GetRange(range, transform))
                range.UnionOf(range, meshRange);
            }
        }
    return range.IsNull() ? ERROR : SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::FlushStale(uint64_t staleTime)
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                Nicholas.Woodfield     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::GetTiles(TileCallback& callback, double resolution) 
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    bool isUnderMaximumSize = resolution < m_info.m_dMax;
    bool hasNoDisplayableChildren = true;

    //Some levels of detail have multiple files and thus we get a node with multiple children that all correspond to the same tile...need to handle getting that geometry and returning it all in a single list
    //rather than separate tiles
    if (!m_primary && m_children.size() > 1)
        {
        hasNoDisplayableChildren = true;
        }
    else
        {
        for (auto const& child : m_children)
            {
            if (resolution >= child->m_info.m_dMax && !child->m_info.m_children.empty())
                {
                //Load children up front, if we have one that has meshes keep loading 
                child->Load();
                if (child->GetMeshCount() > 0)
                    hasNoDisplayableChildren = false;
                }
            }
        }

    if (m_meshes.size() > 0 && !m_info.m_children.empty() && (isUnderMaximumSize || hasNoDisplayableChildren))
        {
        uint32_t tileX, tileY;
        if (MRMeshUtil::ParseTileId(m_info.m_children[0], tileX, tileY) == SUCCESS)
            {
            bvector<bpair<PolyfaceHeaderPtr, int>> geom;
            geom.reserve(m_meshes.size());


            for (auto const& mesh : m_meshes)
                {
                PolyfaceHeaderPtr polyface = mesh->GetPolyface();
                PolyfaceHeaderPtr copy = PolyfaceHeader::CreateVariableSizeIndexed();
                copy->CopyFrom(*polyface);

                int texId = mesh->GetTextureId();
                geom.push_back(bpair<PolyfaceHeaderPtr, int>(copy, texId));
                }

            bvector<bpair<Byte*, Point2d>> textures;
            for (auto const& tex : m_textures)
                {
                ByteCP texData = tex->GetData();
                Point2d size = tex->GetSize();

                Byte* copy = new Byte[size.x * size.y * 4];
                memcpy(copy, texData, size.x * size.y * 4);

                textures.push_back(bpair<Byte*, Point2d>(copy, size));
                }

            callback._OnTile(tileX, tileY, geom, textures);

            geom.clear();
            textures.clear();
            }
        }
    else
        {
        for (auto const& child : m_children)
            child->GetTiles(callback, resolution);
        }

    //Clear all non-primary nodes as we go so we don't load too much and potentially run out of memory
    if (!m_primary)
        Clear();
#endif
    }
