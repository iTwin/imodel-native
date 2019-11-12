/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <bsibasegeomPCH.h>
#include    <Geom/IntegerTypes/BSIRect.h>
#include    <algorithm>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    BSIRect::Limit (BSIRect const* other)
    {
    if (origin.x < other->origin.x)
        origin.x = other->origin.x;
    else if (origin.x > other->corner.x)
        origin.x = other->corner.x;

    if (corner.x < other->origin.x)
        corner.x = other->origin.x;
    else if (corner.x > other->corner.x)
        corner.x = other->corner.x;

    if (origin.y < other->origin.y)
        origin.y = other->origin.y;
    else if (origin.y > other->corner.y)
        origin.y = other->corner.y;

    if (corner.y < other->origin.y)
        corner.y = other->origin.y;
    else if (corner.y > other->corner.y)
        corner.y = other->corner.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    BSIRect::Inset (int deltaX, int deltaY)
    {
    if ((Width() - 2*deltaX) <= 0 || (Height() - 2*deltaY) <= 0)
        {
        Init (0, 0, 0, 0);
        return;
        }

    origin.x += deltaX;
    corner.x -= deltaX;
    origin.y += deltaY;
    corner.y -= deltaY;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void    BSIRect::Expand (int delta)
    {
    if (origin.x < corner.x)
        {
        origin.x -= delta;
        corner.x += delta;
        }
    else
        {
        origin.x += delta;
        corner.x -= delta;
        }

    if (origin.y < corner.y)
        {
        origin.y -= delta;
        corner.y += delta;
        }
    else
        {
        origin.y += delta;
        corner.y -= delta;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSIRect::Offset (int dx, int dy)
    {
    origin.x += dx;
    origin.y += dy;
    corner.x += dx;
    corner.y += dy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
void            BSIRect::Union (BSIRect const* other)
    {
    if (other->origin.x < origin.x)  origin.x = other->origin.x;
    if (other->origin.y < origin.y)  origin.y = other->origin.y;
    if (other->corner.x > corner.x)  corner.x = other->corner.x;
    if (other->corner.y > corner.y)  corner.y = other->corner.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSIRect::IsEqual (BSIRect const* other) const
    {
    return ((origin.x == other->origin.x) && (origin.y == other->origin.y) &&
            (corner.x == other->corner.x) && (corner.y == other->corner.y));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSIRect::PointInside (Point2d const& pt) const
    {
    return (pt.x >= origin.x && pt.x <= corner.x &&
            pt.y >= origin.y && pt.y <= corner.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSIRect::Overlap (BSIRect* overlap, BSIRect const* other) const
    {
    int maxOrgX, maxOrgY, minCrnX, minCrnY;

    maxOrgX = std::max (origin.x, other->origin.x);
    maxOrgY = std::max (origin.y, other->origin.y);
    minCrnX = std::min (corner.x, other->corner.x);
    minCrnY = std::min (corner.y, other->corner.y);

    if ((maxOrgX > minCrnX) || (maxOrgY > minCrnY))
        return false;

    if (overlap)
        {
        overlap->origin.x = maxOrgX;
        overlap->corner.x = minCrnX;
        overlap->origin.y = maxOrgY;
        overlap->corner.y = minCrnY;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BSIRect::IsContained (BSIRect const* container) const
    {
    return      origin.x >= container->origin.x
             && origin.y >= container->origin.y
             && corner.x <= container->corner.x
             && corner.y <= container->corner.y;
    }
