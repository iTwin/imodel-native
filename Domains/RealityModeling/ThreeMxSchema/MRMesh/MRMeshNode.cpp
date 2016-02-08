/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/MRMesh/MRMeshNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"



USING_NAMESPACE_BENTLEY_DGNPLATFORM
                                             
MRMeshNode::~MRMeshNode() { MRMeshCacheManager::GetManager().RemoveRequest (*this); }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshNode::Clear ()
    {
    m_dir.clear();
    m_children.clear();
    m_meshes.clear();
    m_textures.clear();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::_PushJpegTexture (Byte const* data, size_t dataSize)
    {
    m_textures.push_back (MRMeshTexture::Create (data, dataSize));
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::_PushNode(const S3NodeInfo& nodeInfo)
    {
    if (nodeInfo.m_children.empty())
        return;

    m_children.push_back (MRMeshNode::Create (nodeInfo, this));
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::_AddGeometry(int nodeId, int nbVertices,float* positions,float* normals,int nbTriangles,int* indices,float* textureCoordinates,int textureId)
    {
    if (textureId >= 0)
        m_meshes.push_back (MRMeshGeometry::Create (nbVertices, positions, normals, nbTriangles, indices, textureCoordinates, textureId));
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName    MRMeshNode::GetFileName () const  {return m_info.m_children.empty() ? BeFileName() : MRMeshUtil::ConstructNodeName (m_info.m_children.front(), NULL == m_parent ? NULL : &m_parent->m_dir); }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::Load ()
    {
    if (!m_dir.empty())
        return SUCCESS;         // Already loaded.

    if (1 != m_info.m_children.size())
        {
        BeAssert (false);
        return ERROR;
        }
    std::string     err;
    if (SUCCESS != MRMeshCacheManager::GetManager().SynchronousRead (*this, GetFileName()))
        {
        MRMeshUtil::DisplayNodeFailureWarning (GetFileName().c_str());
        m_dir.clear();
        return ERROR;
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::LoadUntilDisplayable ()
    {
    static  uint32_t    s_sleepMillis = 20;
//  uint32_t            startTicks = mdlSystem_getTicks();
//  bool                loadingMessageDisplayed = false;

    while (!LoadedUntilDisplayable ())
        {
        static int s_pauseTicks;

        MRMeshCacheManager::GetManager().ProcessRequests();
        RequestLoadUntilDisplayable();
        BeThreadUtilities::BeSleep (s_sleepMillis);

#ifdef NEEDS_WORK_LOAD_NOTIFY
        if (!loadingMessageDisplayed &&
            (mdlSystem_getTicks() - startTicks) > s_notifyTicks)
            {
            WString     message;

            RmgrResource::LoadWString (message, g_rfHandle, STRINGLISTID_Misc, MISC_Loading);

            loadingMessageDisplayed = true;
            NotificationManager().OutputMessage (NotifyMessageDetails (OutputMessagePriority::Info, message.c_str()));
            }
#endif
        }

#ifdef NEEDS_WORK_LOAD_NOTIFY
     if (loadingMessageDisplayed)
        {
        WString     message;

        RmgrResource::LoadWString (message, g_rfHandle, STRINGLISTID_Misc, MISC_LoadingComplete);

        NotificationManager().OutputMessage (NotifyMessageDetails (OutputMessagePriority::Info, message.c_str()));
        }
#endif
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool  MRMeshNode::LoadedUntilDisplayable () const
    {
    if (!IsLoaded())
        return false;

    if (IsDisplayable())
        return true;

    for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        if (! (*child)->LoadedUntilDisplayable ())
            return false;

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  MRMeshNode::RequestLoadUntilDisplayable ()
    {
    if (IsLoaded() && !IsDisplayable())
        {
        m_primary = true;           // Mark this as a primary node (will never get flushed).

        if (!m_childrenRequested)
            {
            m_childrenRequested = true;
            MRMeshCacheManager::GetManager().QueueChildLoad (m_children, NULL, Transform::FromIdentity());
            }
        else
            {
            for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
                (*child)->RequestLoadUntilDisplayable();
            }
        }
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool MRMeshNode::AreChildrenLoaded () const
    {
    for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        if (!(*child)->IsLoaded())
            return false;

    return true;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshNode::Validate (MRMeshNodeCP parent) const
    {
    for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        if (!(*child)->Validate (this))
            return false;

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshNode::ReleaseQVisionCache ()
    {
    for (auto& mesh : m_meshes)
        mesh->ReleaseQVisionCache ();
        
    for (auto& texture : m_textures)
        texture->ReleaseQVisionCache ();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshNode::IsCached () const
    {
    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        if (!(*mesh)->IsCached ())
            return false;

    return true;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshNode::AreVisibleChildrenLoaded (T_MeshNodeArray& unloadedVisibleChildren, ViewContextR viewContext, MRMeshContextCR meshContext) const
    {
    ClipVectorCP        clip = viewContext.GetTransformClipStack().GetClip();

    if (meshContext.GetLoadSynchronous())
        {
        // If the LOD filter is off we assume that this is an application that is interested in full detail (and isn't going to wait for nodes to load.
        // We synchronously load unloaded but visible nodes and release non-visible nodes.
        for (auto& child : m_children)
            {
            if (child->IsDisplayable())
                {
                if (NULL == clip || clip->PointInside (child->m_info.m_center, child->m_info.m_radius))
                    child->Load();
                else if (!child->m_primary)
                    child->Clear();
                }
            }
        return true;
        }
    else
        {
        for (auto& child : m_children)
            if (!child->IsLoaded() &&
                 child->IsDisplayable() &&
                (NULL == clip || clip->PointInside (child->m_info.m_center, child->m_info.m_radius)))
                unloadedVisibleChildren.push_back (child);

        return unloadedVisibleChildren.empty();
        }
    }



/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawMeshes (ViewContextR viewContext, MRMeshContextCR MeshContext)
    {
    if (!IsLoaded())
        {
        BeAssert (false);
        return;
        }

    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        (*mesh)->Draw (viewContext, *this, MeshContext);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawBoundingSphere (ViewContextR context)
    {
    Render::GraphicPtr graphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport()));

    graphic->AddSolidPrimitive (*ISolidPrimitive::CreateDgnSphere(DgnSphereDetail (m_info.m_center, m_info.m_radius)));
    graphic->Close();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    context.OutputGraphic(*graphic, nullptr); // When is this called, should this be added to the scene, etc?!?
#endif
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d MRMeshNode::GetRange () const
    {
    DRange3d range;
    range.InitFrom (m_info.m_center);
    range.Extend (m_info.m_radius);
    return range;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::DrawMeshes (Render::GraphicR graphic, TransformCR transform)
    {
    if (SUCCESS != Load ())
        return;

    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        {
        PolyfaceHeaderPtr       polyface = PolyfaceHeader::CreateVariableSizeIndexed();

        polyface->CopyFrom (*(*mesh)->GetPolyface());
        polyface->Transform (transform);
        graphic.AddPolyface (*polyface, false);
        }
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MRMeshNode::TestVisibility (bool& isUnderMaximumSize, ViewContextR viewContext, MRMeshContextCR meshContext)
    {
    ClipVectorCP        clip;

    if (NULL != (clip = viewContext.GetTransformClipStack().GetClip()) &&
        !clip->PointInside (m_info.m_center, m_info.m_radius))
            return false;

    if (meshContext.UseFixedResolution())
        {
        isUnderMaximumSize = (m_info.m_radius / meshContext.GetFixedResolution()) < m_info.m_dMax;
        }
    else
        {
        double          pixelSize  =  m_info.m_radius / viewContext.GetPixelSizeAtPoint (&m_info.m_center);

        isUnderMaximumSize = pixelSize < MRMeshUtil::CalculateResolutionRatio() * m_info.m_dMax;
        }
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::Draw (bool& childrenScheduled, ViewContextR viewContext, MRMeshContextCR meshContext)
    {
   bool                isUnderMaximumSize = false, childrenNeedLoading = false;

    if (!TestVisibility (isUnderMaximumSize, viewContext, meshContext) || !IsLoaded())
        return SUCCESS;

    m_lastUsed = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

    T_MeshNodeArray     unloadedVisibleChildren;
    
    if ((!isUnderMaximumSize || !IsDisplayable()) && 
        ! m_children.empty() && 
        ! (childrenNeedLoading = !AreVisibleChildrenLoaded (unloadedVisibleChildren, viewContext, meshContext)))
        {                                                                                                 
        for (bvector<MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
            {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
            if (DrawPurpose::Update != viewContext.GetDrawPurpose() && viewContext.CheckStop())      // Only check stop when not updating  -- doing it during update can leave incomplete display.
#else
            if (viewContext.CheckStop())      // Only check stop when not updating  -- doing it during update can leave incomplete display.
#endif
                return SUCCESS;

            (*child)->Draw (childrenScheduled, viewContext, meshContext);
            }
        return SUCCESS;
        }

    DrawMeshes (viewContext, meshContext);

    if (meshContext.GetLoadSynchronous() &&         // If we just are exporting fixed resolution clear here to mimimize memory usage.
        meshContext.UseFixedResolution() &&
        !m_primary)
        Clear();

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if ((DrawPurpose::Update  == viewContext.GetDrawPurpose() || DrawPurpose::UpdateHealing == viewContext.GetDrawPurpose()) &&
        !isUnderMaximumSize &&
        childrenNeedLoading &&
        NULL != viewContext.GetViewport())
        {
        childrenScheduled = true;
        MRMeshCacheManager::GetManager().QueueChildLoad (unloadedVisibleChildren, viewContext.GetViewport(), meshContext.GetTransform());
        }
#endif

    return SUCCESS;
    }



/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MRMeshNode::DrawCut (ViewContextR viewContext, MRMeshContextCR meshContext, DPlane3dCR plane)
    {
    bool                isUnderMaximumSize = false;

    if (!TestVisibility (isUnderMaximumSize, viewContext, meshContext) || !IsLoaded())
        return SUCCESS;

    T_MeshNodeArray     unloadedVisibleChildren;
    
    if ((!isUnderMaximumSize || !IsDisplayable()) && 
        ! m_children.empty() && 
        AreVisibleChildrenLoaded (unloadedVisibleChildren, viewContext, meshContext))
        {                                                                                                 
        for (bvector<MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
            {
            if (viewContext.CheckStop())
                return SUCCESS;

            (*child)->DrawCut (viewContext, meshContext, plane);
            }
        return SUCCESS;
        }

    for (auto& mesh : m_meshes)
        mesh->DrawCut (viewContext, plane);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshNode::GetMeshCount () const
    {
    size_t      count = m_meshes.size();

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        count += (*child)->GetMeshCount();

    return count;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshNode::GetMeshMemorySize () const
    {
    size_t      memorySize = 0;

    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        memorySize += (*mesh)->GetMemorySize ();

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        memorySize += (*child)->GetMeshMemorySize();

    return memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshNode::GetTextureMemorySize () const
    {
    size_t      memorySize = 0;

    for (bvector<MRMeshTexturePtr>::const_iterator texture = m_textures.begin (); texture != m_textures.end (); texture++)
        memorySize += (*texture)->GetMemorySize();

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        memorySize += (*child)->GetTextureMemorySize();

    return memorySize;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshNode::GetNodeCount () const
    {
    size_t      count = IsLoaded() ? 1 : 0;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        count += (*child)->GetNodeCount ();

    return count;
    }
/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t      MRMeshNode::GetMaxDepth () const
    {
    size_t maxChildDepth = 0, childDepth;

    for (bvector<MRMeshNodePtr>::const_iterator child = m_children.begin (); child != m_children.end (); child++)
        if ((childDepth = (*child)->GetMaxDepth ()) > maxChildDepth)
            maxChildDepth = childDepth;

    return 1 + maxChildDepth;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void    MRMeshNode::RemoveChild (MRMeshNodeP failedChild)
    {
    for (bvector <MRMeshNodePtr>::iterator child = m_children.begin(); child != m_children.end(); child++)
        {
        if (child->get() == failedChild)
            {
            m_children.erase (child);
            break;
            }
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void        MRMeshNode::ClearQvElems ()
    {
    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        (*mesh)->ClearQvElems ();

    for (bvector <MRMeshNodePtr>::iterator child = m_children.begin(); child != m_children.end(); child++)
        (*child)->ClearQvElems ();
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void        MRMeshNode::ClearQvElemReferences ()
    {
    for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin (); mesh != m_meshes.end (); mesh++)
        (*mesh)->ClearQvElemReferences ();

    for (bvector <MRMeshNodePtr>::iterator child = m_children.begin(); child != m_children.end(); child++)
        (*child)->ClearQvElemReferences ();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MRMeshNodePtr        MRMeshNode::Create (S3NodeInfo const& info, MRMeshNodeP parent) { return new MRMeshNode (info, parent); }

MRMeshNodePtr        MRMeshNode::Create ()
    {
    S3NodeInfo      nodeInfo;

    nodeInfo.m_center.Zero();
    nodeInfo.m_radius = 1.0E10;
    nodeInfo.m_dMax   = 0.0;
    return new MRMeshNode (nodeInfo, NULL);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                        Grigas.Petraitis            04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::Clone (MRMeshNode const& other)
    {
    BeAssert(&other != this);
    m_parent    = other.m_parent;
    m_info      = other.m_info;
    m_dir       = other.m_dir;
    m_meshes    = other.m_meshes;
    m_textures  = other.m_textures;
    m_children.clear();

    for (bvector <MRMeshNodePtr>::const_iterator otherChild = other.m_children.begin(); otherChild != other.m_children.end(); otherChild++)
        {
        MRMeshNodePtr child = MRMeshNode::Create();
        child->Clone(**otherChild);
        child->m_parent = this;
        m_children.push_back(child);
        }
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  MRMeshNode::GetDepthMap (bvector<size_t>& map, bvector <bset<BeFileName>>& fileNames, size_t depth)
    {
    if (!IsLoaded())
        return;

    if (map.size() <= depth)
        map.push_back (1);
    else
        map[depth]++;

    
    if (fileNames.size() <= depth)
        {
        bset<BeFileName>    thisNames;

        thisNames.insert (GetFileName());
        fileNames.push_back (thisNames);
        }
    else
        {
        if (fileNames[depth].find (GetFileName()) != fileNames[depth].end())
            {
            BeAssert (false && ">>>>>>>>>>>>>>>>>>>>>Duplicate<<<<<<<<<<<<<<<<<<<<<<<<");
            }
        fileNames[depth].insert (GetFileName());
        }
        

    for (auto& child : m_children)
        child->GetDepthMap (map, fileNames, depth+1);
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MRMeshNode::GetRange (DRange3dR range, TransformCR transform) const
    {
    range = DRange3d::NullRange();

    for (auto const& child : m_children)     // Prefer the more accurate child range...
        {
        DRange3d        childRange = DRange3d::NullRange();

        if (SUCCESS == child->GetRange (childRange, transform))
            range.UnionOf (range, childRange);
        }

    if (range.IsNull())
        {
        for (auto const& mesh : m_meshes)
            {
            DRange3d        meshRange = DRange3d::NullRange();

            if (SUCCESS == mesh->GetRange (range, transform))
                range.UnionOf (range, meshRange);
            }
        }
    return range.IsNull() ? ERROR : SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::FlushStale (uint64_t staleTime)
    {
    if (m_lastUsed < staleTime)
        {
        Clear ();
        }
    else
        {
        for (bvector <MRMeshNodePtr>::iterator child = m_children.begin (); child != m_children.end (); child++)
            (*child)->FlushStale (staleTime);
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                Nicholas.Woodfield     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MRMeshNode::GetTiles(GetTileCallback callback, double resolution)
    {
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
        for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin(); child != m_children.end(); child++)
            {
            if (resolution >= (*child)->m_info.m_dMax && !(*child)->m_info.m_children.empty())
                {
                //Load children up front, if we have one that has meshes keep loading 
                (*child)->Load();
                if ((*child)->GetMeshCount() > 0)
                    hasNoDisplayableChildren = false;
                }
            }
        }

    if(m_meshes.size() > 0 && !m_info.m_children.empty() && (isUnderMaximumSize || hasNoDisplayableChildren))
        {
        uint32_t tileX, tileY;
        if (MRMeshUtil::ParseTileId(m_info.m_children[0], tileX, tileY) == SUCCESS)
            {
            bvector<bpair<PolyfaceHeaderPtr, int>> geom;
            geom.reserve(m_meshes.size());

            bvector<bpair<Byte*, Point2d>> textures;
            textures.reserve(m_textures.size());

            for (bvector<MRMeshGeometryPtr>::const_iterator mesh = m_meshes.begin(); mesh != m_meshes.end(); mesh++)
                {
                PolyfaceHeaderCP polyface = (*mesh)->GetPolyfaceCP();
                PolyfaceHeaderPtr copy = PolyfaceHeader::CreateVariableSizeIndexed();
                copy->CopyFrom(*polyface);

                int texId = (*mesh)->GetTextureId();

                geom.push_back(bpair<PolyfaceHeaderPtr, int>(copy, texId));
                }

            for (bvector<MRMeshTexturePtr>::const_iterator tex = m_textures.begin(); tex != m_textures.end(); tex++)
                {
                ByteCP texData = (*tex)->GetData();
                Point2d size = (*tex)->GetSize();

                Byte* copy = new Byte[size.x * size.y * 4];
                memcpy(copy, texData, size.x * size.y * 4);

                textures.push_back(bpair<Byte*, Point2d>(copy, size));
                }

            callback(tileX, tileY, geom, textures);

            geom.clear();
            textures.clear();
            }
        }
    else
        {
        for (bvector <MRMeshNodePtr>::const_iterator child = m_children.begin(); child != m_children.end(); child++)
            {
            (*child)->GetTiles(callback, resolution);
            }
        }

    //Clear all non-primary nodes as we go so we don't load too much and potentially run out of memory
    if (!m_primary)
        Clear();
    }






















