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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Dump(Utf8CP header) const
    {
    puts(header);
    printf("Children=%s, nGeom=%d\n", GetChildFile().c_str(), (int) m_geometry.size());

    Utf8String childHdr = Utf8String(header) + "  ";
    ChildNodes const* children = GetChildren();
    if (nullptr == children)
        return;

    for (auto const& child : *children)
        child->Dump(childHdr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Draw this node. If it is too coarse, instead draw its children if they are loaded. 
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
            UnloadChildren(args.m_purgeOlderThan); 
            return;
            }

        double radius = args.m_scene.GetNodeRadius(*this);
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
    if (tooCoarse && nullptr != children) // this node is too coarse for current view, don't draw it and instead draw its children
        {
        m_childrenLastUsed = args.m_now;

        for (auto const& child : *children)
            child->Draw(args, depth+1);

        return;
        }

    if (!m_geometry.empty()) // if we have geometry, draw it now
        {
        for (auto geom : m_geometry)
            geom->Draw(args);
        }

    if (!HasChildren()) // this is a visible leaf node - we're done
        return;

    if (!tooCoarse) // if this node was fine enough for the current zoom scale, but it has children,  then they're no longer needed, unload them if they're loaded
        {
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

    // this is actually a race condtion, but it doesn't matter. If the loading thread misses the abandoned flag, the only consequence is we waste a little time.
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
    DgnDb::VerifyClientThread();
    if (ChildLoad::Ready != m_childLoad.load()) // children aren't loaded, nothing to do
        return;

    if (m_childrenLastUsed > olderThan) // have we used this node's children recently?
        {
        // this node has been used recently. We're going to keep it, but potentially unload its grandchildren
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr Geometry::GetPolyface() const
    {
    Graphic::TriMeshArgs trimesh;
    trimesh.m_numIndices = (int32_t) m_indices.size();
    trimesh.m_vertIndex = m_indices.empty() ? nullptr : &m_indices.front();
    trimesh.m_numPoints = (int32_t) m_points.size();
    trimesh.m_points = m_points.empty() ? nullptr : &m_points.front();
    trimesh.m_normals = m_normals.empty() ? nullptr: &m_normals.front();
    trimesh.m_textureUV = m_textureUV.empty() ? nullptr : &m_textureUV.front();
    return trimesh.ToPolyface();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(Graphic::TriMeshArgs const& args, SceneR scene)
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

    if (nullptr != args.m_textureUV)
        {
        m_textureUV.resize(args.m_numPoints);
        memcpy(&m_textureUV.front(), args.m_textureUV, args.m_numPoints * sizeof(FPoint2d));
        }

    if (nullptr == scene.GetRenderSystem() || !args.m_texture.IsValid())
        return;

    m_graphic = scene.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams());
    m_graphic->SetSymbology(ColorDef::White(), ColorDef::White(), 0);
    m_graphic->AddTriMesh(args);
    m_graphic->Close();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Geometry::GetMemorySize() const
    {
    size_t size = m_points.size() * sizeof(FPoint3d) + m_normals.size() * sizeof(FPoint3d) + m_textureUV.size() * sizeof(FPoint2d);

    if (m_graphic.IsValid())
        {
        size += (m_points.size()  * 3 +
                 m_normals.size() * 3 +
                 m_textureUV.size() * 2) * sizeof(float);     // Account for QVision data (floats).
        }

    return size;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Geometry::Draw(DrawArgsR args)
    {
    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Geometry::GetRange(TransformCR trans) const    
    {
    DRange3d range = DRange3d::NullRange();

    for (auto const &fPoint : m_points)
        {
        DPoint3d dPt = {fPoint.x, fPoint.y, fPoint.z};
        range.Extend(trans, &dPt, 1);
        }

    return range;
    }
