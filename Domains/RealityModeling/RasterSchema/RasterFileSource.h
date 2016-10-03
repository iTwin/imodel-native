/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__


BEGIN_BENTLEY_RASTER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterFileSource : RasterRoot
{
protected:
    RasterFilePtr       m_rasterFile;
    RasterFileModel&    m_model;
    Transform           m_physicalToSource;  //! Transformation from raster file true origin to a lower-left origin. In pixel unit.    
    DMap4d              m_physicalToWorld;   //! Including units change and reprojection approximation if any.

    folly::Future<BentleyStatus> _RequestTile(TileTree::TileCR tile, TileTree::TileLoadsPtr loads) override;

    RasterFileSource(RasterFileR rasterFile, RasterFileModel& model, Dgn::Render::SystemP system);
    ~RasterFileSource();

public:
    static RasterFileSourcePtr Create(Utf8StringCR resolvedName, RasterFileModel& model, Dgn::Render::SystemP system);

    Render::Image QueryTile(TileId const& id, bool& alphaBlend);

    DMatrix4dCR GetPhysicalToWorld() const { return m_physicalToWorld.M0; }
    DMatrix4dCR GetWorldToPhysical() const { return m_physicalToWorld.M1; }
};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterFileTile : RasterTile
{
protected:
    
public:
    typedef RasterFileSource root_type;

    RasterFileTile(RasterFileSourceR root, TileId id, RasterFileTileCP parent);

    BentleyStatus _LoadTile(Dgn::TileTree::StreamBuffer&, Dgn::TileTree::RootR) override { BeAssert(!"not expected we load from _RequestTile"); return ERROR; }

    TileTree::Tile::ChildTiles const* _GetChildren(bool load) const override;
};


END_BENTLEY_RASTER_NAMESPACE