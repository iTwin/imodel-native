/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "RasterTileTree.h"

USING_NAMESPACE_DGN_CESIUM

//---------------------------------------------------------------------------------------------
//-------------------------------- RasterRoot -------------------------------------------------
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterRoot::RasterRoot(RasterModel& model, Utf8CP rootUrl)
    : Cesium::TriMeshTree::Root(model.GetDgnDb(), Transform::FromIdentity(), rootUrl), m_model(model)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
void RasterRoot::GenerateResolution(bvector<Resolution>& resolution, uint32_t width, uint32_t height, uint32_t tileSizeX, uint32_t tileSizeY)
    {
    resolution.push_back(Resolution(width, height, tileSizeX, tileSizeY));

    // Add the resolution until we have reach the lower limits.
    while (height > tileSizeY || width > tileSizeX)
        {
        // Compute the lower resolution.
        width = (uint32_t) ceil(width / 2.0);
        height = (uint32_t) ceil(height / 2.0);

        resolution.push_back(Resolution(width, height, tileSizeX, tileSizeY));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorCP RasterRoot::_GetClipVector() const
    {
    return m_model.GetClip().GetClipVector();
    }

//---------------------------------------------------------------------------------------------
//-------------------------------- RasterTile -------------------------------------------------
//---------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* Construct a new MapTile by TileId.
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RasterTile::RasterTile(RasterRootR root, TileId const& id, RasterTileCP parent)
    : Tile(root, parent), m_id(id)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Utf8String RasterTile::_GetName() const
    {
    return Utf8PrintfString("%d/%d/%d", m_id.resolution, m_id.x, m_id.y);
    }

