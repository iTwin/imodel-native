/*--------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/ThreeMxNode.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ThreeMxInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Node::GetChildFile() const
    {
    Utf8String parentPath("/");
    if (m_parent)
        parentPath = m_parent->GetChildFile();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Dump(Utf8CP header) const
    {
    puts(header);
    printf("Children=%s, nGeom=%d\n", GetChildFile().c_str(), m_geometry.IsValid());

    Utf8String childHdr = Utf8String(header) + "  ";
    ChildNodes const* children = GetChildren();
    if (nullptr == children)
        return;

    for (auto const& child : *children)
        child->Dump(childHdr.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::Draw(DrawArgsR args, int depth)
    {
    bool tooCoarse = true;

    if (IsDisplayable())
        {
        Frustum box(m_range);
        args.m_scene.GetLocation().Multiply(box.GetPtsP(), box.GetPts(), 8); 

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box))
            {
            UnloadChildren();  // NEEDS_WORK - maybe only do this if we need memory, maybe add lru field?
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

    ChildNodes const* children = GetChildren();
    if (tooCoarse && nullptr != children) // this node is too coarse for current view, don't draw it and instead draw its children
        {
        for (auto const& child : *children)
            child->Draw(args, depth+1);

        return;
        }

    if (m_geometry.IsValid())
        m_geometry->Draw(args);

    if (!tooCoarse || !HasChildren())
        {
        UnloadChildren(); // NEEDS_WORK - maybe only do this if we need memory?
        return;
        }

    // this node is too coarse (we drew it already, of course) but has unloaded children. Put it into the list of "missing tiles" so we'll draw its children when they're loaded
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
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Node::UnloadChildren()
    {
    DgnDb::VerifyClientThread();
    if (ChildLoad::Ready != m_childLoad.load()) // nothing to do
        return;

    for (auto const& child : m_childNodes)
        child->SetAbandoned();

    m_childLoad.store(ChildLoad::NotLoaded);
    m_childNodes.clear();
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

