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
    m_nodePath.clear();
    m_children.clear();
    m_geometry.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetFilePath(SceneR scene) const
    {
    BeAssert(!m_info.m_childPath.empty());

    Utf8String parentPath;
    if (m_parent)
        parentPath = m_parent->m_nodePath;

    return scene.ConstructNodeName(m_info.m_childPath, parentPath);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::LoadChildren(SceneR scene)
    {
    if (!m_nodePath.empty())
        return SUCCESS;         // Already loaded.

    Utf8String parentPath;
    if (m_parent)
        parentPath = m_parent->m_nodePath;

    Utf8String myFile =  GetFilePath(scene);
    if (SUCCESS != scene.SynchronousRead(*this, myFile))
        {
        scene.DisplayNodeFailureWarning(myFile);
        m_nodePath.clear();
        return ERROR;
        }

    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::LoadedUntilDisplayable(SceneR scene) 
    {
    if (IsDisplayable())
        return true;

    if (!AreChildrenLoaded())
        {
        if (!m_childrenRequested)
            scene.QueueLoadChildren(*this, nullptr);

        return false;
        }

    bool allLoaded = true;
    for (auto const& child : m_children)
        {
        if (!child->LoadedUntilDisplayable(scene))
            allLoaded = false;
        }

    return allLoaded;
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
    for (auto const& geometry : m_geometry)
        {
        if (!geometry->IsCached())
            return false;
        }

    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawGeometry(RenderContextR context)
    {
    for (auto const& geom : m_geometry)
        geom->Draw(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Dump(Utf8CP header) const
    {
    puts(header);
    printf("Children=%s, file=%s, nGeom=%d\n", m_info.m_childPath.c_str(), m_nodePath.c_str(), (int) m_geometry.size());

    Utf8String childHdr = Utf8String(header) + "  ";
    for (auto const& child : m_children)
        child->Dump(childHdr.c_str());
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawBoundingSphere(RenderContextR context, SceneCR scene) const
    {
    Render::GraphicPtr graphic = context.CreateGraphic(Render::Graphic::CreateParams());

    ColorDef white=ColorDef::White();
    white.SetAlpha(50);
    graphic->SetSymbology(white, white, 0);
    graphic->AddSolidPrimitive(*ISolidPrimitive::CreateDgnSphere(DgnSphereDetail(scene.GetNodeCenter(m_info), scene.GetNodeRadius(m_info))));
    graphic->Close();
    context.OutputGraphic(*graphic, nullptr);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::TestVisibility(ViewContextR viewContext, SceneR scene)
    {
    if (!IsDisplayable())
        return true; // this seems wierd, but "is displayable" really means its a root node with no max diameter. That means we need to draw its children.

    DPoint3d center = scene.GetNodeCenter(m_info);
    double radius = scene.GetNodeRadius(m_info);
    return viewContext.IsPointVisible(center, ViewContext::WantBoresite::No, radius);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::Draw(RenderContextR context, SceneR scene)
    {
    if (!TestVisibility(context, scene))
        return false;

    double radius = scene.GetNodeRadius(m_info);
    bool tooCoarse = true;

    if (IsDisplayable())
        {
        if (scene.UseFixedResolution())
            {
            tooCoarse = (radius / scene.GetFixedResolution()) > m_info.GetMaxDiameter();
            }
        else
            {
#if defined (BENTLEYCONFIG_OS_WINODWS) || defined(BENTLEYCONFIG_OS_WINRT)
            DPoint3d center = scene.GetNodeCenter(m_info);
            double pixelSize  =  radius / context.GetPixelSizeAtPoint(&center);
            tooCoarse = pixelSize > Scene::CalculateResolutionRatio() * m_info.GetMaxDiameter();
#else
                tooCoarse = false;
#endif
            }
        }

    if (tooCoarse && !m_children.empty()) // this node is too coarse for current view, don't draw it and instead draw its children
        {
        bool childrenScheduled = false;
        for (auto const& child : m_children)
            childrenScheduled |= child->Draw(context, scene);

        return childrenScheduled;
        }

    DrawGeometry(context);
    if (!tooCoarse || m_childrenRequested || !HasChildren())
        return false;

    scene.QueueLoadChildren(*this, context.GetViewport());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetMeshCount() const
    {
    size_t count = m_geometry.size();

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

    for (auto const& mesh : m_geometry)
        memorySize += mesh->GetMemorySize();

    for (auto const& child : m_children)
        memorySize += child->GetMeshMemorySize();

    return memorySize;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetNodeCount() const
    {
    size_t count = 1;

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
    for (auto const& mesh : m_geometry)
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
    m_parent = other.m_parent;
    m_info   = other.m_info;
    m_nodePath = other.m_nodePath;
    m_geometry = other.m_geometry;
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
DRange3d Node::GetRange(TransformCR trans) const
    {
    DRange3d range = DRange3d::NullRange();

    for (auto const& child : m_children)     // Prefer the more accurate child range...
        range.UnionOf(range, child->GetRange(trans));

    if (range.IsNull())
        {
        for (auto const& mesh : m_geometry)
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

