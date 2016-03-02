/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ImageUtilities.h>

#ifdef DEBUG_MERCATOR
#   define DEBUG_PRINTF THREADLOG.debugv
#   define DEBUG_ERRORLOG THREADLOG.errorv
#else
#   define DEBUG_PRINTF(fmt, ...)
#   define DEBUG_ERRORLOG(fmt, ...)
#endif

#define WEBMERCATOR_COMPARE_VALUES(val0, val1)  if (val0 < val1) {return true;} if (val0 > val1) {return false;}

BEGIN_UNNAMED_NAMESPACE
static bool s_drawSubstitutes = true;

struct CompareDPoint3dLess
    {
    bool operator() (DPoint3dCR l, DPoint3dCR r) const 
        {
        WEBMERCATOR_COMPARE_VALUES(l.x,r.x);
        WEBMERCATOR_COMPARE_VALUES(l.y,r.y);
        WEBMERCATOR_COMPARE_VALUES(l.z,r.z);
        return false;
        }
    };

struct CompareGeoPointLess
    {
    bool operator() (GeoPointCR l, GeoPointCR r) const 
        {
        WEBMERCATOR_COMPARE_VALUES(l.latitude,r.latitude);
        WEBMERCATOR_COMPARE_VALUES(l.longitude,r.longitude);
        WEBMERCATOR_COMPARE_VALUES(l.elevation,r.elevation);
        return false;
        }
    };

enum {MIN_ZOOM_LEVEL=0, MAX_ZOOM_LEVEL=22};

//=======================================================================================
// Dislays tiles of a street map as they become available over time.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorDisplay : ProgressiveTask
{
    friend struct WebMercatorModel;

protected:
    WebMercatorModel const& m_model;
    DgnUnits& m_units;
    bvector<WebMercatorModel::TileId> m_missingTilesToBeRequested;    //!< Tiles that are missing and that should be requested
    bmap<Utf8String, WebMercatorModel::TileId> m_missingTilesPending; //!< Tiles that are missing and that we are waiting for.
    uint32_t m_failedAttempts = 0;
    uint64_t m_nextRetryTime = 0;                             //!< When to re-try m_missingTilesPending. unix millis UTC
    uint64_t m_waitTime = 0;                                  //!< How long to wait before re-trying m_missingTilesPending. millis 
    uint8_t m_optimalZoomLevel = 0;                           //!< The zoomLevel that would be optimal for the view. Not necessarily used by every tile.
    bool m_hadError = false;                                    //!< If true, no tiles could be found or displayed.
    bool m_drawSubstitutes = s_drawSubstitutes;
    bool m_preferFinerResolution = false;                       //!< Download and display more tiles, in order to get the best resolution? Else, go with 1/4 as many tiles and be satisfied with slightly fuzzy resolution.
    bool m_drawingSubstituteTiles = false;
    double m_originLatitudeInRadians;
    bmap<DPoint3d, GeoPoint, CompareDPoint3dLess> m_metersToLatLng;
    bmap<GeoPoint, DPoint3d, CompareGeoPointLess> m_latLngToMeters;
    ByteStream m_rgbBuffer;

protected:
    bool IsValid() const {return m_units.GetDgnGCS() != NULL;}
    BentleyStatus ComputeTileCorners(DPoint3d* corners, WebMercatorModel::TileId const& tileid);
    double ComputeGroundResolutionInMeters(uint8_t zoomLevel);
    GeoPoint ConvertMetersToLatLng(DPoint3dCR pt);
    DPoint3d ConvertLatLngToMeters(GeoPoint gp);
    DPoint2d ConvertMetersToWpixels(DPoint2dCR pt);
    static uint8_t GetZoomLevels() {return (uint8_t)(MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL);}
    BentleyStatus GetOptimalZoomLevelForView(uint8_t& zoomLevel, DgnViewportR, bool preferFinerResolution);
    BentleyStatus GetTileIdsForView(bvector<WebMercatorModel::TileId>& tileids, uint8_t desiredZoomLevel, DgnViewportR);
    Render::TexturePtr DefineTexture(RgbImageInfo const& imageInfo, RenderContext& context);
    static Render::TextureP GetCachedTexture(RgbImageInfo& cachedImageInfo, Utf8StringCR url);
    void DrawTile(ViewContextR context, WebMercatorModel::TileId const& tileid, RgbImageInfo const& imageInfo, Render::TextureR textureId);
    void DrawMissingTile(ViewContextR context, WebMercatorModel::TileId const& tileid);
    void DrawAndCacheTile(RenderContext& context, WebMercatorModel::TileId const& tileid, Utf8StringCR url, TiledRaster& realityData);
    BentleyStatus DrawSubstituteTilesFinerFromTextureCache(ViewContextR context, WebMercatorModel::TileId const& tileid, uint32_t maxLevelsToTry);
    BentleyStatus DrawCoarserTilesForViewFromTextureCache(ViewContextR context, uint8_t zoomLevel, uint32_t maxLevelsToTry);

    struct TileDisplayImageData
        {
        WebMercatorModel::TileId m_tileid;
        RgbImageInfo m_imageInfo;
        Render::TexturePtr m_texture;
        };

    BentleyStatus GetCachedTiles(bvector<TileDisplayImageData>& tilesAndUrls, bool& allFoundInTextureCache, uint8_t zoomLevel, ViewContextR context);
    virtual Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
    void DrawView(ViewContextR);
    WebMercatorDisplay(WebMercatorModel const& model, DgnViewportR vp);
};

/*---------------------------------------------------------------------------------**//**
* Conversions used in the Web Mercator projection and tiling system.
* See https://developers.google.com/maps/documentation/javascript/examples/map-coordinates for the formulas used here.
* "World" means pixel coordinates in the 256x256 tile that covers the entire map (i.e., tile #0).
* "Pixel" means "pixelCoordinate = worldCoordinate * 2^zoomLevel"
* See https://developers.google.com/maps/documentation/javascript/maptypes#WorldCoordinates
*
* Also see http://wiki.openstreetmap.org/wiki/Zoom_levels
*
* "The latitude and longitude are assumed to be on the WGS 84 datum."
* source: http://msdn.microsoft.com/en-us/library/bb259689.aspx
*
* "The longitude is assumed to range from -180 to +180 degrees, 
* and the latitude must be clipped to range from -85.05112878 to 85.05112878. This avoids a 
* singularity at the poles, and it causes the projected map to be square."
* source: http://msdn.microsoft.com/en-us/library/bb259689.aspx
* This also goes for Google maps and OpenStreetMaps.
*
* Useful to read: 
* http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/
* http://www.codeproject.com/Articles/463434/GoogleMapsNet-GoogleMaps-Control-for-NET
*
*   TERMINOLOGY SHIFT
* In here, to avoid confusion, we change a few of the terms:
*   "world coordinates" ---> "wpixel" 
*   "world point" ---> "wpixel point" 
*
* Note that wpixels are doubles
*           pixels are UInt32s
*           tileids are   "
*
* Max size of a pixel coordinate is 2^22 * 256 = 2^22 * 2^8 = 2^30
* Max size of a tile coordiante is 2^22
*
+---------------+---------------+---------------+---------------+---------------+------*/

static int const TILE_SIZE = 256;

struct Upoint2d
    {
    uint32_t    x;
    uint32_t    y;
    };

static Upoint2d s_pixelOrigin = {TILE_SIZE / 2, TILE_SIZE / 2};
static double s_pixelsPerLonDegree = TILE_SIZE / 360.0;
static double s_pixelsPerLonRadian = TILE_SIZE / msGeomConst_2pi;

static double s_minLatitude  =  -85.05112878;
static double s_maxLatitude  =   85.05112878;
static double s_minLongitude = -180.0;
static double s_maxLongitude =  180.0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static double bound(double value, double opt_min, double opt_max) 
    {
    value = std::max(value, opt_min);
    value = std::min(value, opt_max);
    return value;
    }

#ifndef NDEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isZoomLevelInRange(uint8_t zoomLevel)
    {
    return zoomLevel <= MAX_ZOOM_LEVEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isGeoPointInRange(GeoPointCR latLng)
    {
    return (s_minLatitude  <= latLng.latitude   && latLng.latitude  <= s_maxLatitude
        &&  s_minLongitude <= latLng.longitude  && latLng.longitude <= s_maxLongitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isWpixelPointInRange(DPoint2dCR point)
    {
    return (0 <= point.x && point.x <= TILE_SIZE) && (0 <= point.y && point.y <= TILE_SIZE);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void clampGeoPoint(GeoPointR latLng)
    {
    if (latLng.latitude < s_minLatitude)
        latLng.latitude = s_minLatitude;
    if (latLng.latitude > s_maxLatitude)
        latLng.latitude = s_maxLatitude;
    if (latLng.longitude < s_minLongitude)
        latLng.longitude = s_minLongitude;
    if (latLng.longitude > s_maxLongitude)
        latLng.longitude = s_maxLongitude;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void clampWpixelCoordinate(double& v)
    {
    if (v < 0)
        v = 0;
    if (BeNumerical::Compare(v, TILE_SIZE) >= 0)
        v = (TILE_SIZE - 1.0e-9);   // fudge the greatest possible coordinate as being just a little inside the edge.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void clampWpixelPoint(DPoint2dR point)
    {
    clampWpixelCoordinate(point.x);
    clampWpixelCoordinate(point.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d fromGeoToWpixelPoint(GeoPointCR latLng) 
    {
    BeAssert(isGeoPointInRange(latLng));

    DPoint2d point;
        
    point.x = s_pixelOrigin.x + latLng.longitude * s_pixelsPerLonDegree;

    // Truncating to 0.9999 effectively limits latitude to 89.189. This is
    // about a third of a tile past the edge of the world tile.
    auto siny = bound(sin(Angle::DegreesToRadians(latLng.latitude)), -0.9999, 0.9999);
    point.y = s_pixelOrigin.y + 0.5 * log((1 + siny) / (1 - siny)) * -s_pixelsPerLonRadian;

    clampWpixelPoint(point);

    return point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GeoPoint fromWpixelToGeoPoint(DPoint2dCR point) 
    {
    BeAssert(isWpixelPointInRange(point));

    GeoPoint gpt;
    gpt.longitude = (point.x - s_pixelOrigin.x) / s_pixelsPerLonDegree;
    auto latRadians = (point.y - s_pixelOrigin.y) / -s_pixelsPerLonRadian;
    gpt.latitude = Angle::RadiansToDegrees(2 * atan(exp(latRadians)) - msGeomConst_piOver2);
    gpt.elevation = 0.0; // caller must fill this in, if there's a real elevation
    return gpt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d fromWpixelToPixelPoint(DPoint2dCR wpixels, uint8_t zoomLevel)
    {
    BeAssert(isZoomLevelInRange(zoomLevel));
    BeAssert(isWpixelPointInRange(wpixels));
    
    uint32_t numTiles = 1 << zoomLevel;
    return DPoint2d::From(wpixels.x * numTiles, wpixels.y * numTiles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Upoint2d fromPixelPointToTileCoordinates(DPoint2dCR pixelPoint)
    {
    Upoint2d tileid;
    tileid.x = (decltype(tileid.x)) floor(pixelPoint.x / TILE_SIZE);
    tileid.y = (decltype(tileid.y)) floor(pixelPoint.y / TILE_SIZE);
    return tileid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorModel::TileId fromWpixelPointToTileId(DPoint2dCR wpixelPoint, uint8_t zoomLevel)
    {
    BeAssert(isZoomLevelInRange(zoomLevel));
    Upoint2d xy = fromPixelPointToTileCoordinates(fromWpixelToPixelPoint(wpixelPoint, zoomLevel));
    WebMercatorModel::TileId tileid;
    tileid.column = xy.x;
    tileid.row    = xy.y;
    tileid.zoomLevel =  zoomLevel;
    return tileid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d fromTileIdToWpixelPoint(WebMercatorModel::TileId const& tileid)
    {
    auto pix = tileid.column * TILE_SIZE;
    auto piy = tileid.row    * TILE_SIZE;

    DPoint2d wpixelPoint;
    uint32_t numTiles = 1 << tileid.zoomLevel;
    wpixelPoint.x = pix / (double)numTiles;
    wpixelPoint.y = piy / (double)numTiles;
    
    return wpixelPoint;
    }

//=======================================================================================
// hold the most recent 200 textures in memory, identified by their tile URL
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct TextureCache
{
    struct Entry 
    {
        static uint64_t Next() {static uint64_t s_count=0; return ++s_count;}
        mutable uint64_t m_accessed;
        Render::TexturePtr  m_texture;
        RgbImageInfo m_imageInfo;
        void Accessed() const {m_accessed = Next();}
    };

    struct TileSpec
    {
        DgnModelId m_model;

    };
    bmap<Utf8String, Entry> m_map;

    void Insert(Utf8StringCR url, RgbImageInfo const& info, Render::TextureR texture)
        {
        Trim();

        auto& entry = m_map[url];

        entry.m_texture = &texture;
        entry.m_imageInfo = info;
        entry.Accessed();
        }

    Entry const* Get(Utf8StringCR url)
        {
        auto ifound = m_map.find(url);
        return (ifound == m_map.end())? nullptr: &ifound->second;
        }

    void Trim()
        {
        if (m_map.size() < 200)
            return;
        BeAssert(m_map.size() == 200);

        // find the oldest entry and remove it.
        auto oldest = m_map.begin();
        auto oldestTime = oldest->second.m_accessed;
        for (auto it=++oldest; it!=m_map.end(); ++it)
            {
            if (it->second.m_accessed < oldestTime)
                {
                oldest = it;
                oldestTime = it->second.m_accessed;
                }
            }
        m_map.erase(oldest);
        }
};

static TextureCache s_textureCache;

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus projectPointToPlaneInDirection(DPoint3dR pointOnPlane, DPlane3dCR plane, DPoint3dCR pt, DVec3dCR dir)
    {
    DRay3d ray;
    ray.origin = pt;
    ray.direction = dir;
    double param;
    return ray.Intersect(pointOnPlane, param, plane)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Compute meters/pixel at the Project's geo origin the specified zoomLevel.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
double WebMercatorDisplay::ComputeGroundResolutionInMeters(uint8_t zoomLevel)
    {
    // "Exact length of the equator (according to wikipedia) is 40075.016686 km in WGS-84. A horizontal tile size at zoom 0 would be 156543.034 meters" (http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale)
    return 156543.034 * cos(m_originLatitudeInRadians) / (1<<zoomLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeoPoint WebMercatorDisplay::ConvertMetersToLatLng(DPoint3dCR dp)
    {
    auto i = m_metersToLatLng.find(dp);
    if (i != m_metersToLatLng.end())
        return i->second;

    GeoPoint gp;
    m_units.LatLongFromXyz(gp, dp);

    m_metersToLatLng[dp] = gp;
    m_latLngToMeters[gp] = dp;

    return gp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d WebMercatorDisplay::ConvertLatLngToMeters(GeoPoint gp)
    {
    auto i = m_latLngToMeters.find(gp);
    if (i != m_latLngToMeters.end())
        return i->second;

    DPoint3d dp;
    m_units.XyzFromLatLong(dp, gp);

    m_latLngToMeters[gp] = dp;
    m_metersToLatLng[dp] = gp;

    return dp;
    }

/*---------------------------------------------------------------------------------**//**
* Convert a DPoint2d expressed in the Project's UOR coordinate system to a DPoint2d expressed in wpixels.
* Note that this function can fail, if pt is outside the WebMercator box. In that case, 
* an out-of-range point is clamped to the edge of the box.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d WebMercatorDisplay::ConvertMetersToWpixels(DPoint2dCR pt)
    {
    auto pt3 = DPoint3d::From(pt.x,pt.y,0);
    auto latlng = ConvertMetersToLatLng(pt3);
    clampGeoPoint(latlng);
    return fromGeoToWpixelPoint(latlng);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::ComputeTileCorners(DPoint3d* corners, WebMercatorModel::TileId const& tileid)
    {
    WebMercatorModel::TileId t11(tileid);
    t11.row = t11.column = 1;
    auto t11Wpixels = fromTileIdToWpixelPoint(t11);
    auto tileSizeWpixels = t11Wpixels.x;

    // We want 4 corners in the wpixel coordinate system that Google maps uses
    //    ----x----->
    //  | [0]     [1]
    //  y            
    //  | [2]     [3]
    //  v
    DPoint2d cornersWpixels[4];
    cornersWpixels[0] = fromTileIdToWpixelPoint(tileid);    // returns upper left corner of tile
    cornersWpixels[1].SumOf(cornersWpixels[0], DPoint2d::From(tileSizeWpixels,  0              ));
    cornersWpixels[2].SumOf(cornersWpixels[0], DPoint2d::From(0,                tileSizeWpixels));
    cornersWpixels[3].SumOf(cornersWpixels[0], DPoint2d::From(tileSizeWpixels,  tileSizeWpixels));

    for (size_t i=0; i<_countof(cornersWpixels); ++i)
        {
        corners[i] = ConvertLatLngToMeters(fromWpixelToGeoPoint(cornersWpixels[i]));
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureP WebMercatorDisplay::GetCachedTexture(RgbImageInfo& cachedImageInfo, Utf8StringCR url)
    {
    auto existingTexture = s_textureCache.Get(url);
    if (existingTexture == nullptr)
        return nullptr;

    existingTexture->Accessed(); // update lru
    cachedImageInfo = existingTexture->m_imageInfo;
    return existingTexture->m_texture.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TexturePtr WebMercatorDisplay::DefineTexture(RgbImageInfo const& imageInfo, RenderContext& context)
    {
    BeAssert(!imageInfo.m_isBGR);
    
    Render::ImagePtr image = new Render::Image(imageInfo.m_width, imageInfo.m_height, imageInfo.m_hasAlpha ? Render::Image::Format::Rgba : Render::Image::Format::Rgb, m_rgbBuffer.GetData(), m_rgbBuffer.GetSize());
    return context.GetTargetR().CreateTileSection(*image, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawTile(ViewContextR context, WebMercatorModel::TileId const& tileid, RgbImageInfo const& imageInfo, Render::TextureR texture)
    {
    // Get 4 corners in this order:
    //  [0]     [1]
    //             
    //  [2]     [3]
    DPoint3d uvPts[4];
    ComputeTileCorners(uvPts, tileid);

    if (!imageInfo.m_isTopDown)
        {
        std::swap(uvPts[0], uvPts[2]);
        std::swap(uvPts[1], uvPts[3]);
        }

    // Make sure the map displays beneath element graphics. Note that this policy is appropriate for the background map, which is always
    // "on the ground". It is not appropriate for other kinds of reality data, even some images display. It is up to the individual reality
    // data handler to use the "surface" that is appprpriate to the reality data.
    if (!context.GetViewport()->GetViewController().GetTargetModel()->Is3d())
        {
        for (auto& pt : uvPts)
            pt.z = -DgnViewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority
        }
    else
        {
        auto extents = context.GetViewport()->GetViewController().GetViewedExtents();
        for (auto& pt : uvPts)
            pt.z = extents.low.z - 1;
        }

    GraphicPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport()));
    graphic->AddTile(texture, uvPts);     
    context.OutputGraphic(*graphic, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawAndCacheTile(RenderContext& context, WebMercatorModel::TileId const& tileid, Utf8StringCR url, TiledRaster& realityData)
    {
    auto const& data = realityData.GetData();
    RgbImageInfo imageInfo = realityData.GetImageInfo();
    Utf8String const& contentType = realityData.GetContentType();
    
    BentleyStatus status;

    m_rgbBuffer.Clear(); // reuse the same buffer, in order to minimize mallocs

    if (contentType.Equals("image/png"))
        {
        status = imageInfo.ReadImageFromPngBuffer(m_rgbBuffer, data.GetData(), data.GetSize());
        if (SUCCESS != status)
            LOG.warningv("Invalid png image data: %s", url.c_str());
        }
    else if (contentType.Equals("image/jpeg"))
        {
        status = imageInfo.ReadImageFromJpgBuffer(m_rgbBuffer, data.GetData(), data.GetSize());
        if (SUCCESS != status)
            LOG.warningv("Invalid jpeg image data: %s", url.c_str());
        }
    else
        {
        BeAssert(false && "Unsupported image type");
        LOG.warningv("Unsupported image type: %s -> %s", url.c_str(), contentType.c_str());
        status = BSIERROR;
        }

    if (SUCCESS != status)
        return;

    auto texture = DefineTexture(imageInfo, context);
    s_textureCache.Insert(url, imageInfo, *texture);
    DrawTile(context, tileid, imageInfo, *texture);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawMissingTile(ViewContextR context, WebMercatorModel::TileId const& tileid)
    {
#ifdef WEBMERCATOR_DEBUG_TILES
    RgbColorDef color = {100,100,100};
    setSymbology(context, color, 215, 0);
    DrawTileAsBox(context, tileid, -DgnViewport::GetDisplayPriorityFrontPlane(), false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::GetOptimalZoomLevelForView(uint8_t& zoomLevel, DgnViewportR vp, bool preferFinerResolution)
    {
    // Get the number of meters / pixel that we are showing in the view
    double viewResolution = vp.GetPixelSizeAtPoint(nullptr); 

    // Return the zoom level that has about the same "ground resolution", which is defined as meters / pixel.
    for (uint8_t i=0; i<=MAX_ZOOM_LEVEL; ++i)
        {
        double gr = ComputeGroundResolutionInMeters(i);
        if (BeNumerical::Compare(gr, viewResolution) <= 0)
            {
            zoomLevel = i;
            if (zoomLevel > 0 && !preferFinerResolution)
                --zoomLevel;
            return BSISUCCESS; 
            }
        }

    // No tile has a enough resolution. Use the finest one.
    zoomLevel = MAX_ZOOM_LEVEL;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::GetTileIdsForView(bvector<WebMercatorModel::TileId>& tileids, uint8_t zoomLevel, DgnViewportR vp)
    {
    ViewControllerCR vc = vp.GetViewController();

    // Get upper left and lower right of view in WebMercator "world coordinates" (i.e., pixels at zoomLevel 0)
    // NB: The origin of the WebMercator tiling system is in the upper left.
    DPoint2d ul, lr;
        {
        auto range = vp.GetFrustum().ToRange();
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        BSIRect rect = vp.GetViewRect();
        DRange3d::NullRange();
        range.Extend(vp.ViewToWorld(DPoint3d::From(rect.Left(), rect.Top())));
        range.Extend(vp.ViewToWorld(DPoint3d::From(rect.Left(), rect.Bottom())));
        range.Extend(vp.ViewToWorld(DPoint3d::From(rect.Right(), rect.Bottom())));
        range.Extend(vp.ViewToWorld(DPoint3d::From(rect.Right(), rect.Top())));
#endif

        // Get the projection of the view corners onto the x-y plane
        auto viewDir = vc.GetZVector();
        auto xyplane = DPlane3d::FromOriginAndNormal(DPoint3d::FromZero(), DVec3d::From(0, 0, 1.0));
        DPoint3d llptxy, urptxy;
        if (projectPointToPlaneInDirection(llptxy, xyplane, range.low, viewDir) != BSISUCCESS
         || projectPointToPlaneInDirection(urptxy, xyplane, range.high, viewDir) != BSISUCCESS)
            {
            LOG.info("The view is not looking at any part of the x-y plane");
            return BSIERROR;
            }

        // Restate as upper left and lower right
        DPoint2d ulptxy, lrptxy;
        ulptxy.x = std::min(llptxy.x, urptxy.x);
        lrptxy.x = std::max(llptxy.x, urptxy.x);
        ulptxy.y = std::max(llptxy.y, urptxy.y);
        lrptxy.y = std::min(llptxy.y, urptxy.y);

        ul = ConvertMetersToWpixels(ulptxy);
        lr = ConvertMetersToWpixels(lrptxy);
        }

    // Get tileids of upper left and lower right
    clampWpixelPoint(ul);
    clampWpixelPoint(lr);

    WebMercatorModel::TileId ultileid = fromWpixelPointToTileId(ul, zoomLevel);
    WebMercatorModel::TileId lrtileid = fromWpixelPointToTileId(lr, zoomLevel);

    // Get tileids of all tiles in the rectangular area
    for (auto row = ultileid.row; row <= lrtileid.row; ++row)
        {
        for (auto col = ultileid.column; col <= lrtileid.column; ++col)
            {
            WebMercatorModel::TileId tid;
            tid.zoomLevel = zoomLevel;
            tid.column = col;
            tid.row = row;
            tileids.push_back(tid);      
            }
        }    

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool shouldDraw(ViewContextR context)
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Pick:
        case DrawPurpose::Decorate:
        case DrawPurpose::CaptureGeometry:
        case DrawPurpose::FenceAccept:
        case DrawPurpose::RegionFlood:
        case DrawPurpose::FitView:
        case DrawPurpose::ExportVisibleEdges:
        case DrawPurpose::ModelFacet:
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::DrawSubstituteTilesFinerFromTextureCache(ViewContextR context, WebMercatorModel::TileId const& tileid, uint32_t maxLevelsToTry)
    {
    if (tileid.zoomLevel >= MAX_ZOOM_LEVEL)
        return BSIERROR;

    // Get tiles at the next finer resolution to fill in the space defined by the specified tileid

    WebMercatorModel::TileId finerUlTileid(tileid); // The tile in the upper left corner of the tile that we need to fill.
    ++finerUlTileid.zoomLevel;
    finerUlTileid.row <<= 1;
    finerUlTileid.column <<= 1;

    for (uint32_t row=0; row < 2; ++row)
        {
        for (uint32_t col=0; col < 2; ++col)
            {
            WebMercatorModel::TileId finerTileid(finerUlTileid);
            finerTileid.column += col;
            finerTileid.row += row;

            Utf8String finerUrl;
            RgbImageInfo finerImageInfo;
            if (m_model._CreateUrl(finerUrl, finerImageInfo, finerTileid) != BSISUCCESS)
                continue;

            // *** NB: Do not try to read from the RealityDataCache. This method is called from _DrawView, and it must be fast!

            Render::TextureP finerImage = GetCachedTexture(finerImageInfo, finerUrl);
            if (nullptr != finerImage)
                {
                DrawTile(context, finerTileid, finerImageInfo, *finerImage);
                }
            else
                {
                if (maxLevelsToTry > 1)
                    DrawSubstituteTilesFinerFromTextureCache(context, finerTileid, maxLevelsToTry-1);
                }
            }
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::DrawCoarserTilesForViewFromTextureCache(ViewContextR context, uint8_t zoomLevel, uint32_t maxLevelsToTry)
    {
    bvector<TileDisplayImageData> coarserTileDisplayImageData;
    bool allCoarserTilesInTextureCache;
    if (GetCachedTiles(coarserTileDisplayImageData, allCoarserTilesInTextureCache, zoomLevel, context) == BSISUCCESS && allCoarserTilesInTextureCache)
        {
        for (auto const& coarserTileddata : coarserTileDisplayImageData)
            {
            if (coarserTileddata.m_texture.IsValid())
                DrawTile(context, coarserTileddata.m_tileid, coarserTileddata.m_imageInfo, *coarserTileddata.m_texture);
            }
        return BSISUCCESS;
        }
    
    if (maxLevelsToTry == 0 || zoomLevel == 0)
        return BSIERROR;
        
    return DrawCoarserTilesForViewFromTextureCache(context, zoomLevel-1, maxLevelsToTry-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::GetCachedTiles(bvector<TileDisplayImageData>& tileDisplayImageData, bool& allFoundInTextureCache, uint8_t zoomLevel, ViewContextR context)
    {
    allFoundInTextureCache = true;
    bvector<WebMercatorModel::TileId> tileIds;
    
    if (GetTileIdsForView(tileIds, zoomLevel, *context.GetViewport()) != BSISUCCESS)
        return BSIERROR; // we get an error if the ground is not in the view at all. In that case, there are no tiles to draw.

    for (auto tileid  : tileIds)
        {
        TileDisplayImageData data;
        data.m_tileid = tileid;

        Utf8String url;
        if (m_model._CreateUrl(url, data.m_imageInfo, data.m_tileid) != BSISUCCESS)
            return BSIERROR; // ?? when would this ever happen??

        Render::TextureP cachedTexture = GetCachedTexture(data.m_imageInfo, url);
        if (nullptr == cachedTexture)
            allFoundInTextureCache = false;

        data.m_texture = cachedTexture;
        tileDisplayImageData.push_back(data);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    RefCountedPtr<WebMercatorDisplay> display = new WebMercatorDisplay(*this, *context.GetViewport());
    display->DrawView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawView(ViewContextR context)
    {
    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    //  First, determine if we can draw map tiles at all.
    if (!shouldDraw(context) || nullptr == context.GetViewport())
        return;

    if (context.GetViewport()->IsCameraOn())    // *** TBD: Not sure if we can support tiled raster in a perspective view or not. 
        return;                                 // ***      I would at least have to figure out how to compute what portions of the earth are in the view.

    m_hadError = true;

    if (!IsValid())
        return;

    //  Decide what zoom level we should use for a view of this size.
    if (GetOptimalZoomLevelForView(m_optimalZoomLevel, *context.GetViewport(), m_preferFinerResolution) != BSISUCCESS)
        return;

    //  Figure out what tiles will be needed to cover the view.
    bvector<TileDisplayImageData> allTilesToBeDisplayed;
    bool allTilesInTextureCache;
    if (GetCachedTiles(allTilesToBeDisplayed, allTilesInTextureCache, m_optimalZoomLevel, context) != BSISUCCESS)
        return;

    m_hadError = false;

    if (allTilesToBeDisplayed.empty())
        return;

    bool coarserTilesDrawn = false;
    if (!allTilesInTextureCache && m_optimalZoomLevel > 0)
        {
        //  If we are missing any tiles, first draw the whole view at a coarser zoomLevel. That provides a backdrop. We'll fill in the tiles that we do have after this.
        //  We draw the whole view rather than substitute for individual missing tiles because we can use the texture cache to draw the whole view.
        coarserTilesDrawn = (BSISUCCESS == DrawCoarserTilesForViewFromTextureCache(context, m_optimalZoomLevel - 1, 3));
        }

    // Display each tile, if available. NB: Look only in the texture cache! Do not try to read from RealityDataCache!
    for (auto const& tileddata : allTilesToBeDisplayed)
        {
        auto tileid = tileddata.m_tileid;
        if (tileddata.m_texture.IsValid())
            {
            DrawTile(context, tileid, tileddata.m_imageInfo, *tileddata.m_texture);
            }
        else
            {
            //  The image is not immediately available. We'll check the RealityDataCache and possibly request a download during progressive display.
            m_missingTilesToBeRequested.push_back(tileid);

            if (!coarserTilesDrawn)
                DrawMissingTile(context, tileid); // draw something in the space (mainly for debugging)

            // Try to draw substitutes at a finer zoomLevel, if we can find them in the texture cache.
            DrawSubstituteTilesFinerFromTextureCache(context, tileid, 2);
            }
        }

    //  If any tiles were not available, register for progressive display.
    if (!m_missingTilesToBeRequested.empty())
        {
        context.GetViewport()->ScheduleProgressiveTask(*this);
        m_waitTime = 100;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        }
    }

/*---------------------------------------------------------------------------------**//**
* This callback is invoked on a timer during progressive display.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion WebMercatorDisplay::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        return Completion::Aborted;

    if (m_hadError)
        return Completion::Failed;

    // First, see if the RealityDataCache has the missing tiles. If so, draw them. If not, request downloads.
    // NOTE: It might take more than one pass to do this for each missing tile.
    while (!m_missingTilesToBeRequested.empty())
        {
        auto tileid = m_missingTilesToBeRequested.back();
        m_missingTilesToBeRequested.pop_back();
        
        Utf8String url;
        RgbImageInfo expectedImageInfo;
        m_model._CreateUrl(url, expectedImageInfo, tileid);
        RefCountedPtr<TiledRaster> realityData;
        if (RealityDataCacheResult::Success == T_HOST.GetRealityDataAdmin().GetCache().Get<TiledRaster>(realityData, url.c_str(), *TiledRaster::RequestOptions::Create(expectedImageInfo)))
            {
            BeAssert(realityData.IsValid());
            //  The image is available from the cache. Great! Draw it.
            if (!m_model._ShouldRejectTile(tileid, url, *realityData))
                DrawAndCacheTile(context, tileid, url, *realityData);
            }
        else
            {
            // The image is not available from the cache. We requested a download, and so we'll have to wait for the download to finish (if ever). 
            m_missingTilesPending[url] = tileid;
            }

        if (context.CheckStop())
            return Completion::Aborted;
        }

    //  Second, once all missing tiles have been requested, poll for arrivals and draw them.
    //  We'll keep going back over this list until we get them all ... or we hit checkstop.
    for (auto iMissing = m_missingTilesPending.begin(); iMissing != m_missingTilesPending.end();  /* incremented or erased in loop*/ )
        {
        if (context.CheckStop())
            return Completion::Aborted;

        auto const& url = iMissing->first;
        auto const& tileid = iMissing->second;

        // See if the image has arrived in the cache.
        RefCountedPtr<TiledRaster> realityData;
        if (RealityDataCacheResult::Success == T_HOST.GetRealityDataAdmin().GetCache().Get<TiledRaster>(realityData, url.c_str(), *TiledRaster::RequestOptions::Create()))
            {
            if (context.CheckStop())
                return Completion::Aborted;

            //  Yes, we now have the image. Draw it and remove it from the list of missing tiles.
            if (!m_model._ShouldRejectTile(tileid, url, *realityData))
                DrawAndCacheTile(context, tileid, url, *realityData);

            iMissing = m_missingTilesPending.erase(iMissing); 
            }
        else
            {
            ++iMissing; // Leave it in m_missingTilesPending to be retried later.
            }
        }

    if (!m_missingTilesPending.empty())
        {
        ++m_failedAttempts;
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        if (m_failedAttempts > 5 && (m_failedAttempts&1)==0)
            {
            LOG.infov("%d failures. Wait %lld millis\n", m_failedAttempts, m_waitTime);
            }
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }

    wantShow = WantShow::Yes;

    //  Don't report "Finished" unless all missing tiles have been found and displayed.
    return m_missingTilesPending.empty() ? Completion::Finished : Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorDisplay::WebMercatorDisplay(WebMercatorModel const& model, DgnViewportR vp) : m_units(vp.GetViewController().GetDgnDb().Units()), m_model(model)
    {
    GeoPoint centerLatLng = ConvertMetersToLatLng(DPoint3d::FromSumOf(*vp.GetViewOrigin(), *vp.GetViewDelta(), 0.5));
    m_originLatitudeInRadians = Angle::DegreesToRadians(centerLatLng.latitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::CreateOsmUrl(WebMercatorModel::TileId const& tileid) const
    {
    Utf8String url;

    if (!m_mercator.m_mapType.empty() && m_mercator.m_mapType[0] == '0')  // "(c) OpenStreetMap contributors"
        url = Utf8PrintfString("http://a.tile.openstreetmap.org/%d/%d/%d.png",tileid.zoomLevel, tileid.column, tileid.row);
    else // *** For now, use MapQuest for satellite images (just in developer builds) ***
        url = Utf8PrintfString("http://otile1.mqcdn.com/tiles/1.0.0/sat/%d/%d/%d.jpg",tileid.zoomLevel, tileid.column, tileid.row);  // "Portions Courtesy NASA/JPL-Caltech and U.S. Depart. of Agriculture, Farm Service Agency"

    // *** WIP_WEBMERCATOR m_features

    return url;
    }

#define MAPBOX_ACCESS_KEY "pk.eyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0.f7c9GAxz6j10kZvL_2DBHg"
#define MAPBOX_ACCESS_KEY_URI_ENCODED "pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::CreateMapBoxUrl(WebMercatorModel::TileId const& tileid) const
    {
    Utf8String url;

    /*
    @2x.png	2x scale(retina)
    png32	32 color indexed PNG
    png64	64 color indexed PNG
    png128	128 color indexed PNG
    png256	256 color indexed PNG
    jpg70	70 % quality JPG
    jpg80	80 % quality JPG
    jpg90	90 % quality JPG
    */
    char const* format = "png32";

    char const* mapid = (!m_mercator.m_mapType.empty() && m_mercator.m_mapType [0] == '0')? "mapbox.streets": "mapbox.satellite";

    //                                                  m  z  x  y  f
    url = Utf8PrintfString("http://api.mapbox.com/v4/%s/%d/%d/%d.%s?access_token=", 
                                                        mapid, 
                                                           tileid.zoomLevel, tileid.column, tileid.row, 
                                                                    format);

    url += MAPBOX_ACCESS_KEY_URI_ENCODED; // NB: This URI-encoded string must not be included in the sprintf format string!

    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool StreetMapModel::_ShouldRejectTile(WebMercatorModel::TileId const& tileid, Utf8StringCR url, TiledRaster& realityData) const
    {
    if (m_mercator.m_mapService[0] != '0' || m_mercator.m_mapType[0] != '1')
        return false;

    static uint8_t const s_mapbox_x[] =
{ /* contents of the raw PNG file that MapBox sends to represent a satellite image that it does not have. */
0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x04,0x03,0x00,0x00,0x00,0xae,0x5c,0xb5,
0x55,0x00,0x00,0x00,0x0c,0x50,0x4c,0x54,0x45,0x20,0x20,0x20,0x33,0x33,0x33,0x3a,
0x3a,0x3a,0x40,0x40,0x40,0x46,0xb5,0x68,0x53,0x00,0x00,0x00,0xd8,0x49,0x44,0x41,
0x54,0x78,0x9c,0xed,0xd8,0x41,0x0a,0xc2,0x30,0x14,0x45,0x51,0x75,0x05,0x82,0x1b,
0x28,0xb8,0x02,0xdd,0xff,0xe2,0xb4,0x88,0xa8,0xe9,0x4c,0x30,0x17,0xe4,0x9c,0x61,
0x26,0xef,0xf3,0xb4,0x6d,0x92,0xdd,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xbe,0xb3,0xdf,0x2e,0x1d,0xa7,0x0e,0x70,0x5a,0xc6,0x95,
0xc3,0x65,0xea,0x00,0x87,0xeb,0xb8,0x72,0x5e,0xa6,0x0e,0xb0,0xc9,0xdb,0x4e,0xf4,
0x63,0x63,0xe0,0xec,0x02,0xc6,0xc4,0xe9,0x05,0x8c,0x91,0xf3,0x0b,0xf8,0xcc,0x0c,
0x0a,0xf8,0x0c,0x2d,0x0a,0x78,0x4f,0x4d,0x0a,0x78,0x8f,0x6d,0x0a,0x78,0xe5,0x46,
0x05,0xbc,0x82,0xab,0x02,0x9e,0xc9,0x59,0x01,0xcf,0xe8,0xae,0x80,0x47,0x76,0x58,
0xc0,0x23,0xbc,0x2c,0x60,0x4d,0x4f,0x0b,0x58,0x2b,0x68,0x0b,0xb8,0x57,0xd0,0x16,
0xd0,0x0f,0x90,0xff,0x04,0xf5,0x9f,0x30,0x7f,0x0c,0xeb,0x17,0x51,0xfe,0x2a,0xae,
0x3f,0x46,0xf9,0xe7,0xb8,0xde,0x90,0xe4,0x5b,0xb2,0x7a,0x53,0x9a,0x6f,0xcb,0xeb,
0x83,0x49,0x7e,0x34,0xab,0x0f,0xa7,0xf9,0xf1,0xbc,0xbe,0xa0,0xc8,0xaf,0x68,0xf2,
0x4b,0xaa,0xfc,0x9a,0x0e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x80,0xff,0x71,0x03,0xde,0xe5,0x11,0x67,0x0b,0x9c,0x2a,0x72,0x00,0x00,0x00,
0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82
};

        // MapBox
    auto const& data = realityData.GetData();
    if (data.GetSize() == sizeof(s_mapbox_x) && 0==memcmp(data.GetData(), s_mapbox_x, data.GetSize()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StreetMapModel::_CreateUrl(Utf8StringR url, RgbImageInfo& expectedImageInfo, WebMercatorModel::TileId const& tileid) const
    {
    // The usual image format info
    expectedImageInfo.m_height = expectedImageInfo.m_width = 256;
    expectedImageInfo.m_hasAlpha = false;
    expectedImageInfo.m_isBGR = false;
    expectedImageInfo.m_isTopDown = true;

    if (m_mercator.m_mapService.empty())
        {
        BeAssert(false && "missing map service");
        LOG.error("missing map service");
        return BSIERROR;
        }

    if (m_mercator.m_mapService[0] == '0')
        {
        url = CreateMapBoxUrl(tileid);
        //printf ("url=%s\n", url.c_str());
        }
    else
        {
        BeAssert(false && "unrecognized map service");
        LOG.errorv("[%s] is an unrecognized map service", m_mercator.m_mapService.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StreetMapModel::_GetCopyrightMessage() const
    {
    if (m_mercator.m_mapService[0] == '0')
        return "(c) Mapbox, Data ODbL (c) OpenStreetMap contributors";

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getStreetMapServerDescription(dgn_ModelHandler::StreetMap::MapService mapService, dgn_ModelHandler::StreetMap::MapType mapType)
    {
    Utf8String descr;
    switch (mapService)
        {
        case dgn_ModelHandler::StreetMap::MapService::MapBox:
            {
            descr = ("Mapbox");   // *** WIP translate
            if (dgn_ModelHandler::StreetMap::MapType::Map == mapType)
                descr.append(" Map");   // *** WIP translate
            else
                descr.append(" Satellite Images"); // *** WIP translate
            break;
            }

#ifndef NDEBUG
        case dgn_ModelHandler::StreetMap::MapService::OpenStreetMaps:
            {
            descr = ("Open Street Maps");   // *** WIP translate
            if (dgn_ModelHandler::StreetMap::MapType::Map == mapType)
                descr.append(" Map");   // *** WIP translate
            else
                descr.append(" Satellite Images"); // *** WIP translate
            break;
            }
#endif
        }
    return descr;
    }

DGNPLATFORM_REF_COUNTED_PTR(WebMercatorModel)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId dgn_ModelHandler::StreetMap::CreateStreetMapModel(DgnDbR db, MapService mapService, MapType mapType, bool finerResolution)
    {
    //Utf8PrintfString modelName("com.bentley.dgn.StreetMap_%d_%d", mapService, mapType); // *** WIP_STREET_MAP how to make sure name is unique?
    // ANSWER to WIP: create a DgnAuthority and use the namespace
    Utf8String modelName = getStreetMapServerDescription(mapService,mapType).c_str();
    DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "StreetMapModel"));
    BeAssert(classId.IsValid());

    WebMercatorModelPtr model = new StreetMapModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(modelName)));

    WebMercatorModel::Mercator props;
    props.m_mapService = Utf8PrintfString("%d", mapService);
    props.m_mapType = Utf8PrintfString("%d", mapType);
    props.m_finerResolution = finerResolution;
    props.m_range = db.Units().GetProjectExtents(); // The "range" of a map could be the whole world. We'll fall back on the project's full extents.

    model->SetMercator(props);
    model->Insert();
    return model->GetModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::SetMercator(WebMercatorModel::Mercator const& props)
    {
    m_mercator = props;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Mercator::ToJson(Json::Value& v) const
    {
    v["mapService"] = m_mapService.c_str();
    v["mapType"] = m_mapType.c_str();
    v["finerResolution"] = m_finerResolution;
    JsonUtils::DPoint3dToJson(v["RangeLow"], m_range.low);
    JsonUtils::DPoint3dToJson(v["RangeHigh"], m_range.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Mercator::FromJson(Json::Value const& v)
    {
    m_mapService = v["mapService"].asString();
    m_mapType = v["mapType"].asString();
    m_finerResolution = v["finerResolution"].asBool();
    JsonUtils::DPoint3dFromJson(m_range.low, v["RangeLow"]);
    JsonUtils::DPoint3dFromJson(m_range.high, v["RangeHigh"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_WriteJsonProperties(Json::Value& val) const
    {
    m_mercator.ToJson(val["WebMercatorModel"]);
    T_Super::_WriteJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_ReadJsonProperties(Json::Value const& val)
    {
    BeAssert(val.isMember("WebMercatorModel"));
    m_mercator.FromJson(val["WebMercatorModel"]);
    T_Super::_ReadJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool TiledRaster::_IsExpired() const {return DateTime::CompareResult::EarlierThan == DateTime::Compare(GetExpirationDate(), DateTime::GetCurrentTime());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<TiledRaster> TiledRaster::Create() {return new TiledRaster();}

#define TABLE_NAME_TiledRaster  "TiledRaster"
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TiledRasterPrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
    {
    static BeAtomic<bool> s_isPrepared;

    virtual bool _IsPrepared() const override {return s_isPrepared;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis           03/2015
    +---------------+---------------+---------------+---------------+---------------+--*/
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override
        {
        if (db.TableExists(TABLE_NAME_TiledRaster))
            {
            s_isPrepared = true;
            return SUCCESS;
            }

        Utf8CP ddl = "Url CHAR PRIMARY KEY, \
                        Raster BLOB,          \
                        RasterSize INT,       \
                        RasterInfo CHAR,      \
                        ContentType CHAR,     \
                        Created BIGINT,       \
                        Expires BIGINT,       \
                        ETag CHAR";

        if (BeSQLite::BE_SQLITE_OK == db.CreateTable(TABLE_NAME_TiledRaster, ddl))
            {
            s_isPrepared = true;
            return SUCCESS;
            }
        return ERROR;
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis           03/2015
    +---------------+---------------+---------------+---------------+---------------+--*/
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db, double percentage) const override
        {
        CachedStatementPtr sumStatement;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(sumStatement, "select sum(RasterSize) from " TABLE_NAME_TiledRaster))
            return ERROR;

        if (BeSQLite::BE_SQLITE_ROW != sumStatement->Step()) 
            return ERROR;
            
        auto rasterSize = sumStatement->GetValueInt(0);         
        uint64_t overHead = (uint64_t)(rasterSize * percentage) / 100;
    
        CachedStatementPtr selectStatement;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "select RasterSize,Created from " TABLE_NAME_TiledRaster " ORDER BY Created ASC"))
            return ERROR;
    
        uint64_t runningSum = 0;
        while ((runningSum < overHead) &&(BeSQLite::BE_SQLITE_ROW == selectStatement->Step()))
            runningSum += selectStatement->GetValueInt(0);
            
        CachedStatementPtr deleteStatement;
        uint64_t creationDate = selectStatement->GetValueInt64(1);
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_TiledRaster " WHERE Created  <= ? "))
            return ERROR;

        deleteStatement->BindInt64(1, creationDate);
        if (BeSQLite::BE_SQLITE_DONE != deleteStatement->Step())
            return ERROR;
            
        return SUCCESS;
        }
    };
BeAtomic<bool> TiledRasterPrepareAndCleanupHandler::s_isPrepared;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr TiledRaster::_GetDatabasePrepareAndCleanupHandler() const
    {
    return new TiledRasterPrepareAndCleanupHandler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const& options)
    {
    TiledRaster const& other = dynamic_cast<TiledRaster const&>(self);
    m_url = other.m_url;
    m_creationDate = other.m_creationDate;
    m_contentType = other.m_contentType;
    m_rasterInfo = other.m_rasterInfo;
    m_data = other.m_data;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& requestOptions) 
    {
    BeAssert(nullptr != dynamic_cast<RequestOptions const*>(&requestOptions));
    RequestOptions const& options = static_cast<RequestOptions const&>(requestOptions);
    
    if (nullptr == options.GetExpectedImageInfo())
        {
        BeAssert(false);
        return ERROR;
        }

    m_url.AssignOrClear(url);
    m_creationDate = DateTime::GetCurrentTime();

    auto contentTypeIter = header.find("Content-Type");
    if (contentTypeIter == header.end())
        return ERROR;

    m_contentType = contentTypeIter->second.c_str();
    m_rasterInfo = *options.GetExpectedImageInfo();

    m_data.SaveData(&body.front(), (uint32_t) body.size());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)
    {
    BeMutexHolder lock(cs);

    CachedStatementPtr stmt;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Raster,RasterSize,RasterInfo,Created,Expires,ETag, ContentType from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    stmt->ClearBindings();
    stmt->BindText(1, key, BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        m_url = key;

        auto raster     = stmt->GetValueBlob(0);
        auto rasterSize = stmt->GetValueInt(1);
        m_data.SaveData((Byte*) raster, rasterSize);

        m_rasterInfo = DeserializeRasterInfo(stmt->GetValueText(2));
        DateTime::FromUnixMilliseconds(m_creationDate,(uint64_t) stmt->GetValueInt64(3));
        m_contentType = stmt->GetValueText(6);

        DateTime expirationDate;
        DateTime::FromUnixMilliseconds(expirationDate,(uint64_t) stmt->GetValueInt64(4));
        SetExpirationDate(expirationDate);
        SetEntityTag(stmt->GetValueText(5));

        return SUCCESS;
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_Persist(BeSQLite::Db& db, BeMutex& cs) const
    {
    int bufferSize = (int) GetData().GetSize();

    int64_t creationTime = 0;
    if (SUCCESS != GetCreationDate().ToUnixMilliseconds(creationTime))
        return ERROR;

    int64_t expirationDate = 0;
    if (SUCCESS != GetExpirationDate().ToUnixMilliseconds(expirationDate))
        return ERROR;

    BeMutexHolder lock(cs);

    CachedStatementPtr selectStatement;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Url from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    selectStatement->ClearBindings();
    selectStatement->BindText(1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == selectStatement->Step())
        {
        // update
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TiledRaster " SET Expires=?,ETag=? WHERE Url=?"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindInt64(1, expirationDate);
        stmt->BindText(2, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText(3, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }
    else
        {
        // insert
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TiledRaster "(Url,Raster,RasterSize,RasterInfo,Created,Expires,ETag, ContentType) VALUES(?,?,?,?,?,?,?,?)"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindText(1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindBlob(2, GetData().GetData(), bufferSize, BeSQLite::Statement::MakeCopy::No);
        stmt->BindInt(3, bufferSize);
        stmt->BindText(4, SerializeRasterInfo(m_rasterInfo).c_str(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindInt64(5, creationTime);
        stmt->BindInt64(6, expirationDate);
        stmt->BindText(7, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText(8, GetContentType(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TiledRaster::SerializeRasterInfo(RgbImageInfo const& info)
    {
    Json::Value json;
    json["hasAlpha"] = info.m_hasAlpha;
    json["height"] = info.m_height;
    json["width"] = info.m_width;
    json["isBGR"] = info.m_isBGR;
    json["isTopDown"] = info.m_isTopDown;
    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RgbImageInfo TiledRaster::DeserializeRasterInfo(Utf8CP serializedJson)
    {
    Json::Value json;
    Json::Reader reader;
    reader.parse(serializedJson, json);

    RgbImageInfo info;
    info.m_hasAlpha = json["hasAlpha"].asBool();
    info.m_height = json["height"].asInt();
    info.m_width = json["width"].asInt();
    info.m_isBGR = json["isBGR"].asBool();
    info.m_isTopDown = json["isTopDown"].asBool();
    return info;
    }
