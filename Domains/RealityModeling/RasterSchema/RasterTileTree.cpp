/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterTileTree.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
bool RasterTile::TryLowerRes(TileTree::DrawArgsR args, int depth) const
    {
    RasterTile* parent = (RasterTile*) m_parent;
    if (depth <= 0 || nullptr == parent)
        {
        // DEBUG_PRINTF("no lower res");
        return false;
        }

    if (parent->HasGraphics())
        {
        //DEBUG_PRINTF("using lower res %d", depth);
        args.m_loResSubstitutes.Add(*parent->m_graphic);
        return true;
        }

    return parent->TryLowerRes(args, depth - 1); // recursion
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterTile::TryHigherRes(TileTree::DrawArgsR args) const
    {
    for (auto const& child : m_children)
        {
        RasterTile* mapChild = (RasterTile*) child.get();

        if (mapChild->HasGraphics())
            {
            //DEBUG_PRINTF("using higher res");
            args.m_hiResSubstitutes.Add(*mapChild->m_graphic);
            }
        }
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
            args.m_missing.Insert(depth, this);

        TryLowerRes(args, DRAW_COARSER_DELTA);
        TryHigherRes(args);

        return;
        }

    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }


//---------------------------------------------------------------------------------------------
//-------------------------------- RasterProgressive ------------------------------------------
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
RasterProgressive::~RasterProgressive()
    {
    if (nullptr != m_loads) 
        m_loads->SetCanceled();

    DEBUG_PRINTF("Raster progressive destroyed");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ProgressiveTaskPtr RasterRoot::_CreateProgressiveTask(Dgn::TileTree::DrawArgsR args, Dgn::TileTree::TileLoadStatePtr loads) 
    {
    return new RasterProgressive(*this, args.m_missing, loads, args.GetLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                9/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion RasterProgressive::_DoProgressive(RenderListContext& context, WantShow& wantShow)
    {
    auto now = BeTimePoint::Now();
    TileTree::DrawArgs args(context, Transform::FromProduct(m_depthTrans, m_root.GetLocation()), m_root, now, now - m_root.GetExpirationTime());
    args.SetClip(m_root.GetModel().GetClip().GetClipVector());

    DEBUG_PRINTF("Raster progressive %d missing", m_missing.size());

    for (auto const& node : m_missing)
        {
        auto stat = node.second->GetLoadStatus();
        if (stat == TileTree::Tile::LoadStatus::Ready)
            node.second->Draw(args, node.first);        // now ready, draw it (this potentially generates new missing nodes)
        else if (stat != TileTree::Tile::LoadStatus::NotFound)
            args.m_missing.Insert(node.first, node.second);     // still not ready, put into new missing list
        }

    BeAssert(args.m_missing.size() < 200);  // More than 200 hundred tiles is suspicious.

    args.RequestMissingTiles(m_root, m_loads);
    args.DrawGraphics(context);  // the nodes that newly arrived are in the GraphicBranch in the DrawArgs. Add them to the context 

    m_missing.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("Raster after progressive still %d missing", m_missing.size());
    if (m_missing.empty()) // when we have no missing tiles, the progressive task is done.
        {
        //m_loads = nullptr; // for debugging
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + BeDuration::Seconds(1); // once per second
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

