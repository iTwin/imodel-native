/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"

/*---------------------------------------------------------------------------------**//**
* Must be called from client thread because it references "m_parent" which can become invalid
* if that node gets unloaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetChildFile() const
    {
    DgnDb::VerifyClientThread();
    Utf8String parentPath("/");
    if (m_parent)
        parentPath = m_parent->GetChildFile();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

/*-----------------------------------------------------------------------------------**//**
* Count the number of node for this node and all of its children.
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int Node::CountNodes() const
    {
    int count = 1;

    ChildNodes const* children = GetChildren();
    if (nullptr != children)
        {
        for (auto const& child : *children)
            count += child->CountNodes();
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. If it is too coarse, instead draw its children, if they are already loaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Draw(DrawArgsR args, int depth)
    {
    bool tooCoarse = true;

    if (IsDisplayable())    // some nodes are merely for structure and don't have any geometry
        {
        Frustum box(m_range);
        args.m_scene.GetLocation().Multiply(box.GetPtsP(), box.GetPts(), 8);

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box))
            {
            UnloadChildren(args.m_purgeOlderThan);  // this node is completely outside the volume of the frustum. Unload any loaded children if they're expired.
            return;
            }

        double radius = args.m_scene.GetNodeRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        if (args.m_scene.UseFixedResolution())
            {
            tooCoarse = (radius / args.m_scene.GetFixedResolution()) > GetMaxDiameter();
            }
        else
            {
            DPoint3d center = args.m_scene.GetNodeCenter(*this);
            double pixelSize = radius / args.m_context.GetPixelSizeAtPoint(&center);
            tooCoarse = pixelSize > GetMaxDiameter();
            }
        }

    ChildNodes const* children = GetChildren(); // returns nullptr if this node's children are not yet loaded.
    if (tooCoarse && nullptr != children)
        {
        // this node is too coarse for current view, don't draw it and instead draw its children
        m_childrenLastUsed = args.m_now; // save the fact that we've used our children to delay purging them if this node becomes unused

        for (auto const& child : *children)
            child->Draw(args, depth+1);

        return;
        }

    // This node is either fine enough for the current view or has no loaded children. We'll draw it.
    if (!m_geometry.empty()) // if we have geometry, draw it now
        {
        for (auto geom : m_geometry)
            geom->Draw(args);
        }

    if (!HasChildren()) // this is a leaf node - we're done
        return;

    if (!tooCoarse)
        {
        // This node was fine enough for the current zoom scale. If it has loaded children from a previous pass, they're no longer needed.
        UnloadChildren(args.m_purgeOlderThan);
        return;
        }

    // this node is too coarse (even though we already drew it) but has unloaded children. Put it into the list of "missing tiles" so we'll draw its children when they're loaded
    args.m_missing.Insert(depth, this);

    if (AreChildrenNotLoaded()) // only request children if we haven't already asked for them
        args.m_scene.RequestData(this, false, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* This method gets called on the (valid) children of nodes as they are unloaded. Its purpose is to notify the loading
* threads that these nodes are no longer referenced and we shouldn't waste time loading them.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::SetAbandoned()
    {
    if (ChildLoad::Ready == m_childLoad.load()) // if this node's children are loaded, set them as abandoned too (recursively)
        {
        for (auto const& child : m_childNodes)
            child->SetAbandoned();
        }

    // this is actually a race condition, but it doesn't matter. If the loading thread misses the abandoned flag, the only consequence is we waste a little time.
    m_childLoad.store(ChildLoad::Abandoned);
    }

/*---------------------------------------------------------------------------------**//**
* Unload all of the children of this node. Must be called from client thread. Usually this will cause the nodes to become unreferenced
* and therefore deleted. Note that sometimes we can unload a child that is still in the download queue. In that case, it will remain alive until
* it arrives. Set its "abandoned" flag to tell the download thread it can skip it (it will get deleted when the download thread releases its reference to it.)
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::UnloadChildren(uint64_t olderThan)
    {
    if (ChildLoad::Ready != m_childLoad.load()) // children aren't loaded, nothing to do
        return;

    if (m_childrenLastUsed > olderThan) // have we used this node's children recently?
        {
        // yes, this node has been used recently. We're going to keep it, but potentially unload its grandchildren
        for (auto const& child : m_childNodes)
            child->UnloadChildren(olderThan);

        return;
        }

    for (auto const& child : m_childNodes)
        child->SetAbandoned();

    m_childLoad.store(ChildLoad::NotLoaded);
    m_childNodes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAlignedBox3d Node::ComputeRange()
    {
    if (m_range.IsValid())
        return m_range;

    ChildNodes const* children = GetChildren(); // only returns fully loaded children
    if (nullptr != children)
        {
        for (auto const& child : *children)
            m_range.UnionOf(m_range, child->ComputeRange()); // this updates the range of the top level node
        }

    return m_range;
    }

/*---------------------------------------------------------------------------------**//**
* Create a PolyfaceHeader from a Geometry
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr Geometry::GetPolyface() const
    {
    IGraphicBuilder::TriMeshArgs trimesh;
    trimesh.m_numIndices = (int32_t) m_indices.size();
    trimesh.m_vertIndex = m_indices.empty() ? nullptr : &m_indices.front();
    trimesh.m_numPoints = (int32_t) m_points.size();
    trimesh.m_points = m_points.empty() ? nullptr : &m_points.front();
    trimesh.m_normals = m_normals.empty() ? nullptr: &m_normals.front();
    trimesh.m_textureUV = nullptr;
    return trimesh.ToPolyface();
    }

/*-----------------------------------------------------------------------------------**//**
* Construct a Geometry from a TriMeshArgs and a Scene. The scene is necessary to get the Render::System, and this
* Geometry is only valid for that Render::System
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(IGraphicBuilder::TriMeshArgs const& args, SceneR scene)
    {
    // After we create a Render::Graphic, we only need the points/indices/normals for picking.
    // To save memory, only store them if the model is locatable.
    if (scene.IsLocatable())
        {
        m_indices.resize(args.m_numIndices);
        memcpy(&m_indices.front(), args.m_vertIndex, args.m_numIndices * sizeof(int32_t));

        m_points.resize(args.m_numPoints);
        memcpy(&m_points.front(), args.m_points, args.m_numPoints * sizeof(FPoint3d));

        if (nullptr != args.m_normals)
            {
            m_normals.resize(args.m_numPoints);
            memcpy(&m_normals.front(), args.m_normals, args.m_numPoints * sizeof(FPoint3d));
            }
        }

    if (nullptr == scene.GetRenderSystem() || !args.m_texture.IsValid())
        return;

    auto graphic = scene.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams());
    graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
    graphic->AddTriMesh(args);
    graphic->Close();

    m_graphic = graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Geometry::Draw(DrawArgsR args)
    {
    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }
