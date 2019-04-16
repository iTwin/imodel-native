/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

    RasterFileSource(RasterFileR rasterFile, RasterFileModel& model, Dgn::Render::SystemP system);
    ~RasterFileSource();
    Utf8CP _GetName() const override {return "RasterFile";}

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
public:
    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  9/2016
    //=======================================================================================
    struct RasterTileLoader : TileTree::TileLoader
        {
        Render::Image m_image;  // filled by _ReadFromSource

        BentleyStatus DoGetFromSource();

        RasterTileLoader(TileTree::TileR tile, TileTree::TileLoadStatePtr loads, Utf8StringCR cacheKey, Dgn::Render::SystemP renderSys) : TileTree::TileLoader("", tile, loads, cacheKey, renderSys) {}
            
        virtual ~RasterTileLoader(){}
        
        RasterFileSourceR GetFileSource() { return static_cast<RasterFileSourceR>(m_tile->GetRootR()); }

        folly::Future<BentleyStatus> _GetFromSource() override;        
        BentleyStatus _LoadTile() override;
        };
protected:
    
public:
    typedef RasterFileSource root_type;

    RasterFileTile(RasterFileSourceR root, TileId id, RasterFileTileCP parent);

    TileTree::Tile::ChildTiles const* _GetChildren(bool load) const override;

    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) override { return new RasterTileLoader(*this, loads, "", renderSys); }
};

END_BENTLEY_RASTER_NAMESPACE
