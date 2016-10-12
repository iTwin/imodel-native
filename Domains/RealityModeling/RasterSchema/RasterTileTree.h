/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterTileTree.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/TileTree.h>

struct TileQuery;

// On some hardware this the tile size limit.
#define TILESIZE_LIMIT 1024
#define IsPowerOfTwo(X) (((X)&((X)-1))==0)

BEGIN_BENTLEY_RASTER_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct TileId
    {
    TileId(uint32_t r, uint32_t tX, uint32_t tY) : resolution(r), x(tX), y(tY) {}

    uint32_t resolution;            // current level. 0 being the finest resolution.
                                    // Tile position:
                                    //   -----------
                                    //  | 0,0 | 1,0 |
                                    //   -----------
                                    //  | 0,1 | 1,1 |
                                    //   -----------
    uint32_t x;
    uint32_t y;
    };

//=======================================================================================
//! The root of a multi-resolution raster.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterRoot : Dgn::TileTree::Root
{

//=======================================================================================
//! Raster resolution definition. Tile size cannot exceed TILESIZE_LIMIT and must be a power 
//! of two to avoid resampling by QV.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct Resolution
    {
    public:
        Resolution(uint32_t width, uint32_t height, uint32_t tileSizeX, uint32_t tileSizeY)
            :m_width(width), m_height(height), m_tileSizeX(tileSizeX), m_tileSizeY(tileSizeY)
            {
            BeAssert(GetTileSizeX() <= TILESIZE_LIMIT && GetTileSizeY() <= TILESIZE_LIMIT);
            BeAssert(IsPowerOfTwo(GetTileSizeX()) && IsPowerOfTwo(GetTileSizeY()));

            m_tileCountX = (width + (tileSizeX - 1)) / tileSizeX;
            m_tileCountY = (height + (tileSizeY - 1)) / tileSizeY;
            }

        uint32_t GetWidth() const { return m_width; }
        uint32_t GetHeight() const { return m_height; }
        uint32_t GetTileCountX() const { return m_tileCountX; }
        uint32_t GetTileCountY() const { return m_tileCountY; }

        uint32_t GetTileSizeX() const { return m_tileSizeX; }
        uint32_t GetTileSizeY() const { return m_tileSizeY; }

    private:

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_tileCountX;
        uint32_t m_tileCountY;
        uint32_t m_tileSizeX;
        uint32_t m_tileSizeY;
    };

protected:
    RasterModel& m_model;

    bvector<Resolution> m_resolution;

public:
    RasterRoot(RasterModel& model, Utf8CP realityCacheName, Utf8CP rootUrl, Dgn::Render::SystemP system);
    ~RasterRoot() {ClearAllTiles();}

    RasterModel& GetModel() { return m_model; }

    uint32_t GetResolutionCount() const { return (uint32_t) m_resolution.size(); }

    Resolution const& GetResolution(uint32_t res) const { return m_resolution[res]; }

    uint32_t GetWidth() const { return GetResolution(0).GetWidth(); }
    uint32_t GetHeight() const { return GetResolution(0).GetHeight(); }

    //! return tile size. Border tiles are clipped to their effective size in pixels
    uint32_t GetTileSizeX(TileId const& id) const { return MIN(GetResolution(id.resolution).GetTileSizeX(), GetResolution(id.resolution).GetWidth() - GetResolution(id.resolution).GetTileSizeX()*id.x); }
    uint32_t GetTileSizeY(TileId const& id) const { return MIN(GetResolution(id.resolution).GetTileSizeY(), GetResolution(id.resolution).GetHeight() - GetResolution(id.resolution).GetTileSizeY()*id.y); }

    static void GenerateResolution(bvector<Resolution>& resolution, uint32_t width, uint32_t height, uint32_t tileSizeX, uint32_t tileSizeY);

};

//=======================================================================================
//! A raster tile. May or may not have its graphics loaded.
// @bsiclass                                                    Mathieu.Marchand  9/2016
//=======================================================================================
struct RasterTile : Dgn::TileTree::Tile
{
protected:
    TileId m_id;                                            //! tile id 
    Dgn::Render::IGraphicBuilder::TileCorners m_corners;    //! 4 corners of tile, in world coordinates
    RasterRootR m_root;                                     //! the root that loaded this tile.
    Dgn::Render::GraphicPtr m_graphic;                      //! the texture for this tile.

    bool m_reprojected = true;                              //! if true, this tile has been correctly reprojected into world coordinates. Otherwise, it is not displayable.

    double m_maxSize = 362 / 2;                             //! the maximum size, in pixels, for a bounding sphere radius, that this Tile should occupy on the screen
    
public:
    friend TileQuery;
    
    RasterTile(RasterRootR root, TileId const& id, RasterTileCP parent);

    RasterRootR GetRoot() { return m_root; }

    TileId GetTileId() const { return m_id; }

    //! 4 corners of tile, in world coordinates
    Dgn::Render::IGraphicBuilder::TileCorners const& GetCorners() const {return m_corners;}

    bool TryLowerRes(Dgn::TileTree::DrawArgsR args, int depth) const;
    void TryHigherRes(Dgn::TileTree::DrawArgsR args) const;

    bool HasGraphics() const { return IsReady() && m_graphic.IsValid(); }

    bool _HasChildren() const override { return m_id.resolution > 0; }

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
    Dgn::TileTree::TileLoadsPtr m_loads;
    TransformCR m_depthTrans;

    Completion _DoProgressive(Dgn::ProgressiveContext& context, WantShow&) override;
    RasterProgressive(RasterRootR root, Dgn::TileTree::DrawArgs::MissingNodes& nodes, Dgn::TileTree::TileLoadsPtr loads, TransformCR depthTrans) : m_root(root), m_missing(std::move(nodes)), m_loads(loads), m_depthTrans(depthTrans) {}
    ~RasterProgressive();
    };

END_BENTLEY_RASTER_NAMESPACE