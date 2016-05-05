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
    Utf8String parentPath = "/";
    if (m_parent)
        parentPath = m_parent->GetChildFile();

    return parentPath.substr(0, parentPath.find_last_of("/")) + "/" + m_childPath;
    }

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
    puts(header);
    printf("Children=%s, nGeom=%d\n", GetChildFile().c_str(), m_geometry.IsValid());

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

    scene.QueueLoadChildren(*this);
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

