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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetFilePath(SceneR scene) const
    {
    BeAssert(!m_childPath.empty());

    Utf8String parentPath;
    if (m_parent)
        parentPath = m_parent->m_nodePath;

    return scene.ConstructNodeName(m_childPath, parentPath);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Node::LoadChildren(SceneR scene)
    {
    BeAssert(m_childLoad.load() == ChildLoad::Invalid);

    Utf8String parentPath;
    if (m_parent)
        parentPath = m_parent->m_nodePath;

    Utf8String myFile = GetFilePath(scene);
    if (SUCCESS != scene.SynchronousRead(*this, myFile))
        {
        scene.DisplayNodeFailureWarning(myFile);
        m_nodePath.clear();
        return ERROR;
        }

    return SUCCESS;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::DrawGeometry(RenderContextR context)
    {
    if (m_geometry.IsValid())
        m_geometry->Draw(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Dump(Utf8CP header) const
    {
    printf(header);
    printf("Children=%s, file=%s, nGeom=%d\n", m_childPath.c_str(), m_nodePath.c_str(), m_geometry.IsValid());

    Utf8String childHdr = Utf8String(header) + "  ";
    ChildNodes const* children = GetChildren();
    if (nullptr == children)
        return;

    for (auto const& child : *children)
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
    graphic->AddSolidPrimitive(*ISolidPrimitive::CreateDgnSphere(DgnSphereDetail(scene.GetNodeCenter(*this), scene.GetNodeRadius(*this))));
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

    DPoint3d center = scene.GetNodeCenter(*this);
    double radius = scene.GetNodeRadius(*this);
    return viewContext.IsPointVisible(center, ViewContext::WantBoresite::No, radius);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool Node::Draw(RenderContextR context, SceneR scene)
    {
    if (!TestVisibility(context, scene))
        return false;

    double radius = scene.GetNodeRadius(*this);
    bool tooCoarse = true;

    if (IsDisplayable())
        {
        if (scene.UseFixedResolution())
            {
            tooCoarse = (radius / scene.GetFixedResolution()) > GetMaxDiameter();
            }
        else
            {
            DPoint3d center = scene.GetNodeCenter(*this);
            double pixelSize = radius / context.GetPixelSizeAtPoint(&center);
            tooCoarse = pixelSize > GetMaxDiameter();
            }
        }

    ChildNodes const* children = GetChildren();
    if (tooCoarse && nullptr != children) // this node is too coarse for current view, don't draw it and instead draw its children
        {
        bool childrenScheduled = false;
        for (auto const& child : *children)
            childrenScheduled |= child->Draw(context, scene);

        return childrenScheduled;
        }

    DrawGeometry(context);

    if (!tooCoarse || !NeedLoadChildren())
        return false;

    scene.QueueLoadChildren(*this, context.GetViewport());
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Node::GetNodeCount() const
    {
    size_t count = 1;

    ChildNodes const* children = GetChildren();
    if (nullptr != children)
        {
        for (auto const& child : *children)
            count += child->GetNodeCount();
        }

    return count;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::RemoveChild(NodeP failedChild)
    {
    if (!AreChildrenValid())
        return;

    for (auto child = m_childNodes.begin(); child != m_childNodes.end(); ++child)
        {
        if (child->get() == failedChild)
            {
            m_childNodes.erase(child);
            return;
            }
        }
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Node::GetRange(TransformCR trans) const
    {
    DRange3d range = DRange3d::NullRange();

    ChildNodes const* children = GetChildren();
    if (nullptr != children)
        {
        for (auto const& child : *children)     // Prefer the more accurate child range...
            range.UnionOf(range, child->GetRange(trans));
        }

    if (range.IsNull())
        {
        if (m_geometry.IsValid())
            range.UnionOf(range, m_geometry->GetRange(trans));
        }

    return range;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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

#endif
