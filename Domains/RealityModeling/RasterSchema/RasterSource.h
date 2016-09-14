/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

// On some hardware this the tile size limit.
#define TILESIZE_LIMIT 1024
#define IsPowerOfTwo(X) (((X)&((X)-1))==0)

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct TileId
    {
    TileId(uint32_t r, uint32_t tX, uint32_t tY) : resolution(r), x(tX), y(tY){}

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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2015
//----------------------------------------------------------------------------------------
struct RasterSource : RefCountedBase
{
    //! Raster resolution definition. Tile size cannot exceed TILESIZE_LIMIT and must be a power of two.
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

            uint32_t GetWidth() const {return m_width;}
            uint32_t GetHeight() const {return m_height;}
            uint32_t GetTileCountX() const {return m_tileCountX;}
            uint32_t GetTileCountY() const {return m_tileCountY;}

            uint32_t GetTileSizeX() const {return m_tileSizeX;}
            uint32_t GetTileSizeY() const {return m_tileSizeY;}

        private:           

            uint32_t m_width;
            uint32_t m_height;
            uint32_t m_tileCountX;
            uint32_t m_tileCountY;
            uint32_t m_tileSizeX;
            uint32_t m_tileSizeY;
        };

    //! Query for tile data. Null might be returned if an error occurs.
    //! Border tiles are assumed to be clipped to the raster physical extent.
    Dgn::Render::Image QueryTile(TileId const& id, bool& alphaBlend);

    uint32_t GetResolutionCount() const {return (uint32_t)m_resolution.size();}

    Resolution const& GetResolution(uint32_t res) const {return m_resolution[res];}

    bool IsValidTileId(TileId const& id) const;
     
    //! Raster 4 corners in source gcs.
    DPoint3dCR GetCorners() const {return m_corners[0];}

    uint32_t GetWidth() const {return GetResolution(0).GetWidth();}
    uint32_t GetHeight() const {return GetResolution(0).GetHeight();}

    //! return tile size. Border tiles are clipped to their effective size in pixels
    uint32_t GetTileSizeX(TileId const& id) const {return MIN(GetResolution(id.resolution).GetTileSizeX(), GetResolution(id.resolution).GetWidth() - GetResolution(id.resolution).GetTileSizeX()*id.x);}
    uint32_t GetTileSizeY(TileId const& id) const {return MIN(GetResolution(id.resolution).GetTileSizeY(), GetResolution(id.resolution).GetHeight() - GetResolution(id.resolution).GetTileSizeY()*id.y);}
    
    //! Compute tiles corners in Cartesian with a lower-left origin.
    //! [0] [1]
    //! [2] [3]
    void ComputeTileCorners(DPoint3dP pCorners, TileId const& id) const;

    //! Return the raster Gcs. Can be NULL.
    GeoCoordinates::BaseGCSP GetGcsP() { return m_pGcs.get(); }

    //! Transform from source lower-left origin to physical(might be lower-left, upper-left ...) coordinate.
    virtual TransformCR _PhysicalToSource() const = 0;

protected:
    static void GenerateResolution(bvector<Resolution>& resolution, uint32_t width, uint32_t height, uint32_t tileSizeX, uint32_t tileSizeY);

    //&&MM review how we initialize a source georef.  We are taking the info from the model now. cleanup to avoid confusion.
    //! Must be called by child class after construction. BaseGCSP might be NULL
    BentleyStatus Initialize(bvector<Resolution>const& resolution, DMatrix4d physicalToCartesian, GeoCoordinates::BaseGCSP pGcs);
    BentleyStatus Initialize(bvector<Resolution>const& resolution, DPoint3dCP corners, GeoCoordinates::BaseGCSP pGcs);

    void SetGcsP(GeoCoordinates::BaseGCSP pNewGcs) {m_pGcs = pNewGcs/*Hold a ref*/;} 

    virtual Dgn::Render::Image _QueryTile(TileId const& id, bool& alphaBlend) = 0;

    //! default empty constructor. Must call Initialize afterward.
    RasterSource(); 
    virtual ~RasterSource();

private:                    
    bvector<Resolution> m_resolution;   // 0 being the finest resolution.
    DPoint3d m_corners[4];              // in units(Cartesian) of our GCS or assumed coincident if we no GCS
    DMatrix4d m_physicalToCartesian;

    GeoCoordinates::BaseGCSPtr m_pGcs;      // might be NULL.  
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE