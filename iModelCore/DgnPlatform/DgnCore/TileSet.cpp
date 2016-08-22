/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/TileSet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

USING_NAMESPACE_TILESET

/*---------------------------------------------------------------------------------**//**
* Draw this node. If it is too coarse, instead draw its children, if they are already loaded.
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::VisitCompleted Tile::Visit(DrawArgsR args, int depth) const
    {
    bool tooCoarse = true;

    if (IsDisplayable())    // some nodes are merely for structure and don't have any geometry
        {
        Frustum box(m_range);
        args.m_location.Multiply(box.GetPtsP(), box.GetPts(), 8);

        if (FrustumPlanes::Contained::Outside == args.m_context.GetFrustumPlanes().Contains(box))
            {
            _UnloadChildren(args.m_purgeOlderThan);  // this node is completely outside the volume of the frustum. Unload any loaded children if they're expired.
            return VisitCompleted::Yes;
            }

        double radius = args.GetTileRadius(*this); // use a sphere to test pixel size. We don't know the orientation of the image within the bounding box.
        DPoint3d center = args.GetTileCenter(*this);
        double pixelSize = radius / args.m_context.GetPixelSizeAtPoint(&center);
        tooCoarse = pixelSize > GetMaximumScreenSpaceError();
        }

    VisitCompleted completed = VisitCompleted::Yes;
    auto children = _GetChildren(); // returns nullptr if this node's children are not yet loaded.
    if (tooCoarse && nullptr != children)
        {
        // this node is too coarse for current view, don't draw it and instead draw its children
        m_childrenLastUsed = args.m_now; // save the fact that we've used our children to delay purging them if this node becomes unused

        for (auto const& child : *children)
            {
            if (VisitCompleted::Yes != child->Visit(args, depth+1))
                completed = VisitCompleted::No;
            }

        return completed;
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
        return VisitCompleted::Yes;
        }

    // this node is too coarse (even though we already drew it) but has unloaded children. Put it into the list of "missing tiles" so we'll draw its children when they're loaded
    args.m_missing.Insert(depth, this);

    if (AreChildrenNotLoaded()) // only request children if we haven't already asked for them
        args.m_scene.RequestData(this, false, nullptr);
    }


