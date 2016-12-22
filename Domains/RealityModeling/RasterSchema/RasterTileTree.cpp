/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterTileTree.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RasterInternal.h"
#include "RasterTileTree.h"

USING_NAMESPACE_TILETREE

//static const uint32_t DRAW_FINER_DELTA = 2;
static const uint32_t DRAW_COARSER_DELTA = 6;


//---------------------------------------------------------------------------------------------
//-------------------------------- RasterRoot -------------------------------------------------
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterRoot::RasterRoot(RasterModel& model, Utf8CP rootUrl, Dgn::Render::SystemP system)
    :TileTree::Root(model.GetDgnDb(), Transform::FromIdentity(), rootUrl, system),
     m_model(model)
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
Utf8String RasterTile::_GetTileName() const
    {
    return Utf8PrintfString("%d/%d/%d", m_id.resolution, m_id.x, m_id.y);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterTile::_DrawGraphics(TileTree::DrawArgsR args, int depth) const
    {
    if (!m_reprojected)     // if we were unable to reproject this tile, don't try to draw it.
        return;

    if (!IsReady())
        {
        if (!IsNotFound())
            args.InsertMissing(*this);

        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }



