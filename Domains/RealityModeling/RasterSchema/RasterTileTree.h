/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterTileTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterSource.h"
#include <DgnPlatform/TileTree.h>

struct TileQuery; //&&MM keep that struct?

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//=======================================================================================
//! The root of a multi-resolution raster.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterRoot : Dgn::TileTree::Root
{
private:
    DMap4d m_physicalToWorld;                      //! Including units change and reprojection approximation if any. Only used when 4 corners reprojection fails.
    RasterSourcePtr m_source;
    RasterModel& m_model;

    folly::Future<BentleyStatus> _RequestTile(Dgn::TileTree::TileCR tile) override;

public:
    RasterRoot(RasterSourceR source, RasterModel& model, Dgn::Render::SystemP system);
    virtual ~RasterRoot();

    RasterSourceR GetSource() { return *m_source; }

    RasterModel& GetModel() { return m_model; }

    DMatrix4dCR GetPhysicalToWorld() const { return m_physicalToWorld.M0; }
    DMatrix4dCR GetWorldToPhysical() const { return m_physicalToWorld.M1; }
};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterTile : Dgn::TileTree::Tile
{
private:
    TileId m_id;                                            //! tile id 
    Dgn::Render::IGraphicBuilder::TileCorners m_corners;    //! 4 corners of tile, in world coordinates
    RasterRootR m_root;                                     //! the root that loaded this tile.
    bool m_reprojected = false;  /*&&MM todo*/              //! if true, this tile has been correctly reprojected into world coordinates. Otherwise, it is not displayable.
    Dgn::Render::GraphicPtr m_graphic;                      //! the texture for this tile.

    double m_maxSize;                                       //! the maximum size, in pixels, for a bounding sphere radius, that this Tile should occupy on the screen
    
public:
    friend TileQuery;
    
    RasterTile(RasterRootR mapRoot, TileId id, RasterTileCP parent);

    RasterRootR GetRoot() { return m_root; }

    TileId GetTileId() const { return m_id; }

    //! 4 corners of tile, in world coordinates
    Dgn::Render::IGraphicBuilder::TileCorners const& GetCorners() const {return m_corners;}

    bool TryLowerRes(Dgn::TileTree::DrawArgsR args, int depth) const;
    void TryHigherRes(Dgn::TileTree::DrawArgsR args) const;

    bool HasGraphics() const { return IsReady() && m_graphic.IsValid(); }

    BentleyStatus _LoadTile(Dgn::TileTree::StreamBuffer&, Dgn::TileTree::RootR) override;

    bool _HasChildren() const override { return m_id.resolution > 0; }
    ChildTiles const* _GetChildren(bool load) const override;

    void _DrawGraphics(Dgn::TileTree::DrawArgsR, int depth) const override;

    Utf8String _GetTileName() const override;

    virtual double _GetMaximumSize() const override 
        {
        static double s_qualityFactor = 1.15; // 1.0 is full quality.
        return m_maxSize * s_qualityFactor; 
        }
};

//=======================================================================================
// @bsiclass                                                   Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterProgressive : Dgn::ProgressiveTask
    {
    RasterRootR m_root;
    Dgn::TileTree::DrawArgs::MissingNodes m_missing;
    Dgn::TileTree::TimePoint m_nextShow;

    Completion _DoProgressive(Dgn::ProgressiveContext& context, WantShow&) override;
    RasterProgressive(RasterRootR root, Dgn::TileTree::DrawArgs::MissingNodes& nodes) : m_root(root), m_missing(std::move(nodes)) {}
    };

END_BENTLEY_RASTERSCHEMA_NAMESPACE