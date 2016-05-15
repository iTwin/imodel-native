/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/ImageUtilities.h>
#include <DgnPlatform/RealityDataCache.h>

#define DEBUG_MERCATOR
#ifdef DEBUG_MERCATOR
#   define DEBUG_PRINTF THREADLOG.debugv
#   define DEBUG_ERRORLOG THREADLOG.errorv
#else
#   define DEBUG_PRINTF(fmt, ...)
#   define DEBUG_ERRORLOG(fmt, ...)
#endif

#define COMPARE_VALUES(val0, val1)  if (val0 < val1) return true; if (val0 > val1) return false;

using namespace WebMercator;

BEGIN_UNNAMED_NAMESPACE

enum
{
    TILE_SIZE = 256,
    MIN_ZOOM_LEVEL = 0,
    MAX_ZOOM_LEVEL = 22,
    MAX_DB_CACHE_SIZE = 1024*1024*1024, // 1 Gb
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
struct Upoint2d
    {
    uint32_t  x;
    uint32_t  y;
    };

static Upoint2d s_pixelOrigin = {TILE_SIZE / 2, TILE_SIZE / 2};
static double s_pixelsPerLonDegree = TILE_SIZE / 360.0;
static double s_pixelsPerLonRadian = TILE_SIZE / msGeomConst_2pi;
static double s_minLatitude  =  -85.05112878;
static double s_maxLatitude  =   85.05112878;
static double s_minLongitude = -180.0;
static double s_maxLongitude =  180.0;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct TileIdList : bvector<TileId>{};
struct TileList : bvector<TilePtr>{};

DEFINE_POINTER_SUFFIX_TYPEDEFS(WPixelPoint)
DEFINE_POINTER_SUFFIX_TYPEDEFS(LatLongPoint)

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct LatLongPoint : GeoPoint
{
    bool operator<(LatLongPoint const& r) const
        {
        COMPARE_VALUES(latitude,r.latitude);
        COMPARE_VALUES(longitude,r.longitude);
        COMPARE_VALUES(elevation,r.elevation);
        return false;
        }
    LatLongPoint() {}
    LatLongPoint(WPixelPointCR point);

    void Clamp()
        {
        if (latitude < s_minLatitude)
            latitude = s_minLatitude;
        if (latitude > s_maxLatitude)
            latitude = s_maxLatitude;
        if (longitude < s_minLongitude)
            longitude = s_minLongitude;
        if (longitude > s_maxLongitude)
            longitude = s_maxLongitude;
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct WPixelPoint : DPoint2d
{
    static void ClampCoordinate(double& v)
    {
    if (v < 0)
        v = 0;
    if (BeNumerical::Compare(v, TILE_SIZE) >= 0)
        v = (TILE_SIZE - 1.0e-9);   // fudge the greatest possible coordinate as being just a little inside the edge.
    }

    void Clamp() {ClampCoordinate(x); ClampCoordinate(y);}
    WPixelPoint() {x=0; y=0;}
    WPixelPoint(LatLongPointCR latLng);
    WPixelPoint(TileId tileid);
    TileId ToTileId(uint8_t zoomLevel);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct PixelPoint : DPoint2d
{
    PixelPoint(WPixelPointCR wpixels, uint8_t zoomLevel);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct TileRange
{
    WPixelPoint m_upperLeft;
    WPixelPoint m_lowerRight;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct WebMercatorProgressive : ProgressiveTask
{
    TileList m_missingTiles;
    uint64_t m_nextShow = 0;
    
    virtual Completion _DoProgressive(ProgressiveContext& context, WantShow&) override;
    WebMercatorProgressive(TileList& missing) : m_missingTiles(std::move(missing)) {}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct DrawArgs
{
    RenderContextR m_context;
    Render::GraphicArray m_graphics;
    TileList m_missing;

    DrawArgs(RenderContextR context) : m_context(context) {}
    void DrawTile(TileR tile) {m_graphics.Add(*tile.m_graphic);}
    void AddMissingTile(TileR tile) {m_missing.push_back(&tile);}
    void DrawGraphics(double);
};

//=======================================================================================
// Dislays tiles of a map 
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorDisplay
{
protected:
    WebMercatorModel const& m_model;
    DgnUnits& m_units;
    TileRange m_tileRange;
    uint8_t m_zoomLevel = 0;                //!< The zoomLevel for this view
    bool m_preferFinerResolution = false;   //!< Download and display more tiles, in order to get the best resolution? Else, go with 1/4 as many tiles and be satisfied with slightly fuzzy resolution.
    double m_originLatitudeInRadians;
    bmap<LatLongPoint, DPoint3d> m_latLngToMeters;

public:
    bool IsValid() const {return nullptr != m_units.GetDgnGCS();}
    Tile::Corners ComputeCorners(TileId tileid);
    double ComputeGroundResolution(uint8_t zoomLevel);
    LatLongPoint ConvertMetersToLatLng(DPoint3dCR pt);
    DPoint3d ConvertLatLngToMeters(LatLongPoint gp);
    WPixelPoint ConvertMetersToWpixels(DPoint2dCR pt);
    static uint8_t GetZoomLevels() {return (uint8_t)(MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL);}
    bool ComputeZoomLevel(DgnViewportR);
    TileIdList GetTileIds(uint8_t desiredZoomLevel);
    TilePtr GetCachedTile(TileId);
    TilePtr GetLoadedTile(TileId);
    void DrawFinerTiles(DrawArgs&, TileId tileid, uint32_t maxLevelsToTry);
    void DrawCoarserTiles(DrawArgs& context, uint8_t zoomLevel, uint32_t maxLevelsToTry);
    void DrawView(TerrainContextR);
    WebMercatorDisplay(WebMercatorModel const& model, DgnViewportR vp);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static double bound(double value, double opt_min, double opt_max)
    {
    value = std::max(value, opt_min);
    value = std::min(value, opt_max);
    return value;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t Tile::Next() {static uint64_t s_count=0; return ++s_count;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr WebMercatorModel::CreateTile(TileId id, Tile::Corners const& corners, Render::SystemR sys) const
    {
    Tile* tile = new Tile(id, corners);
    m_tileCache.Add(id, tile);
    RequestTile(id, *tile, sys);
    return tile;
    }

/*---------------------------------------------------------------------------------**//**
* if the texture cache is full, drop the oldest entry
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::TileCache::Trim()
    {
    if (m_map.size() < m_maxSize)
        return;
    BeAssert(m_map.size() == m_maxSize);

    // find the oldest entry and remove it.
    auto oldest = m_map.begin();
    auto oldestTime = oldest->second->m_lastAccessed;
    for (auto it=oldest; ++it != m_map.end(); )
        {
        if (it->second->m_lastAccessed < oldestTime)
            {
            oldest = it;
            oldestTime = it->second->m_lastAccessed;
            }
        }

    DEBUG_PRINTF("Dropping tile");
    m_map.erase(oldest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool TileId::operator<(TileId const& rhs) const
    {
    COMPARE_VALUES(m_zoomLevel,rhs.m_zoomLevel)
    COMPARE_VALUES(m_column,rhs.m_column)
    COMPARE_VALUES(m_row,rhs.m_row)
    return false;
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
static bool isLatLongPointInRange(LatLongPointCR latLng)
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
WPixelPoint::WPixelPoint(LatLongPointCR latLng)
    {
    BeAssert(isLatLongPointInRange(latLng));

    x = s_pixelOrigin.x + latLng.longitude * s_pixelsPerLonDegree;

    // Truncating to 0.9999 effectively limits latitude to 89.189. This is
    // about a third of a tile past the edge of the world tile.
    auto siny = bound(sin(Angle::DegreesToRadians(latLng.latitude)), -0.9999, 0.9999);
    y = s_pixelOrigin.y + 0.5 * log((1 + siny) / (1 - siny)) * -s_pixelsPerLonRadian;

    Clamp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
LatLongPoint::LatLongPoint(WPixelPointCR point)
    {
    BeAssert(isWpixelPointInRange(point));

    longitude = (point.x - s_pixelOrigin.x) / s_pixelsPerLonDegree;
    auto latRadians = (point.y - s_pixelOrigin.y) / -s_pixelsPerLonRadian;
    latitude = Angle::RadiansToDegrees(2 * atan(exp(latRadians)) - msGeomConst_piOver2);
    elevation = 0.0; // caller must fill this in, if there's a real elevation
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
PixelPoint::PixelPoint(WPixelPointCR wpixels, uint8_t zoomLevel)
    {
    BeAssert(isZoomLevelInRange(zoomLevel));
    BeAssert(isWpixelPointInRange(wpixels));

    uint32_t numTiles = 1 << zoomLevel;
    x = wpixels.x * numTiles;
    y = wpixels.y * numTiles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TileId WPixelPoint::ToTileId(uint8_t zoomLevel)
    {
    PixelPoint pixelPt(*this, zoomLevel);
    return TileId(zoomLevel, (uint32_t) floor(pixelPt.x / TILE_SIZE), (uint32_t) floor(pixelPt.y / TILE_SIZE));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WPixelPoint::WPixelPoint(TileId tileid)
    {
    auto pix = tileid.m_column * TILE_SIZE;
    auto piy = tileid.m_row    * TILE_SIZE;

    uint32_t numTiles = 1 << tileid.m_zoomLevel;
    x = pix / (double)numTiles;
    y = piy / (double)numTiles;
    }


/*---------------------------------------------------------------------------------**//**
* Compute meters/pixel at the Project's geo origin the specified zoomLevel.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
double WebMercatorDisplay::ComputeGroundResolution(uint8_t zoomLevel)
    {
    // "Exact length of the equator (according to wikipedia) is 40075.016686 km in WGS-84. A horizontal tile size at zoom 0 would be 156543.034 meters" (http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale)
    return 156543.034 * cos(m_originLatitudeInRadians) / (1<<zoomLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
LatLongPoint WebMercatorDisplay::ConvertMetersToLatLng(DPoint3dCR dp)
    {
    LatLongPoint gp;
    m_units.LatLongFromXyz(gp, dp);
    m_latLngToMeters[gp] = dp;
    return gp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d WebMercatorDisplay::ConvertLatLngToMeters(LatLongPoint gp)
    {
    auto i = m_latLngToMeters.find(gp);
    if (i != m_latLngToMeters.end())
        return i->second;

    DPoint3d dp;
    m_units.XyzFromLatLong(dp, gp);
    m_latLngToMeters[gp] = dp;
    return dp;
    }

/*---------------------------------------------------------------------------------**//**
* Convert a DPoint2d expressed in the Project's UOR coordinate system to a DPoint2d expressed in wpixels.
* Note that this function can fail, if pt is outside the WebMercator box. In that case,
* an out-of-range point is clamped to the edge of the box.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WPixelPoint WebMercatorDisplay::ConvertMetersToWpixels(DPoint2dCR pt)
    {
    DPoint3d pt3 = DPoint3d::From(pt.x,pt.y,0);
    LatLongPoint latlng = ConvertMetersToLatLng(pt3);
    latlng.Clamp();
    return WPixelPoint(latlng);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::Corners WebMercatorDisplay::ComputeCorners(TileId tileid)
    {
    TileId t11(tileid);

    t11.m_row = t11.m_column = 1;
    WPixelPoint t11Wpixels(t11);
    auto tileSizeWpixels = t11Wpixels.x;

    // We want 4 corners in the wpixel coordinate system
    //    ----x----->
    //  | [0]     [1]
    //  y
    //  | [2]     [3]
    //  v
    WPixelPoint cornersWpixels[4];
    cornersWpixels[0] = WPixelPoint(tileid);    // returns upper left corner of tile
    cornersWpixels[1].SumOf(cornersWpixels[0], DPoint2d::From(tileSizeWpixels,  0              ));
    cornersWpixels[2].SumOf(cornersWpixels[0], DPoint2d::From(0,                tileSizeWpixels));
    cornersWpixels[3].SumOf(cornersWpixels[0], DPoint2d::From(tileSizeWpixels,  tileSizeWpixels));

    Tile::Corners corners;
    for (size_t i=0; i<_countof(cornersWpixels); ++i)
        {
        corners.m_pts[i] = ConvertLatLngToMeters(LatLongPoint(cornersWpixels[i]));
        corners.m_pts[i].z = m_model.GetGroundBias();
        }

    return corners;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr WebMercatorDisplay::GetCachedTile(TileId id)
    {
    auto existingTile = m_model.GetTileCache().Get(id);
    if (!existingTile.IsValid())
        return nullptr;

    existingTile->Accessed(); // update lru
    return existingTile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TilePtr WebMercatorDisplay::GetLoadedTile(TileId id)
    {
    auto existing = GetCachedTile(id);
    return existing.IsValid() && existing->IsLoaded() ? existing : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WebMercatorDisplay::ComputeZoomLevel(DgnViewportR vp)
    {
    DRay3d viewRay;
    viewRay.direction = vp.GetViewController().GetZVector();

    DPlane3d xyPlane = DPlane3d::FromOriginAndNormal(DPoint3d::FromZero(), DVec3d::From(0, 0, 1.0));
    Frustum  worldFrust = vp.GetFrustum();
    DRange2d xyRange = DRange2d::NullRange();

    for (DPoint3dCR pt : worldFrust.m_pts)
        {
        DPoint3d xyPt;
        viewRay.origin = pt;
        double param;
        if (!viewRay.Intersect(xyPt, param, xyPlane))
            return false;

        xyRange.Extend(xyPt);
        }

    double worldDiag = xyRange.low.Distance(xyRange.high);
    Frustum  viewFrust = vp.GetFrustum(DgnCoordSystem::View);
    double viewDiag = viewFrust.m_pts[NPC_LeftBottomRear].Distance(viewFrust.m_pts[NPC_RightTopRear]);

    DPoint2d upperLeft, lowerRight;
    upperLeft.x  = xyRange.low.x;
    upperLeft.y  = xyRange.high.y;
    lowerRight.x = xyRange.high.x;
    lowerRight.y = xyRange.low.y;

    m_tileRange.m_upperLeft  = ConvertMetersToWpixels(upperLeft);
    m_tileRange.m_lowerRight = ConvertMetersToWpixels(lowerRight);

    // Get the number of meters / pixel that we are showing in the view
    double viewResolution = worldDiag / viewDiag;

    // Return the zoom level that has about the same "ground resolution", which is defined as meters / pixel.
    for (uint8_t i=0; i<=MAX_ZOOM_LEVEL; ++i)
        {
        double gr = ComputeGroundResolution(i);
        if (BeNumerical::Compare(gr, viewResolution) <= 0)
            {
            m_zoomLevel = i;
            if (m_zoomLevel > 0 && !m_preferFinerResolution)
                --m_zoomLevel;
            return true;
            }
        }

    // No tile has a enough resolution. Use the finest one.
    m_zoomLevel = MAX_ZOOM_LEVEL;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TileIdList WebMercatorDisplay::GetTileIds(uint8_t zoomLevel)
    {
    TileId lrtileid = m_tileRange.m_lowerRight.ToTileId(zoomLevel);
    TileId ultileid = m_tileRange.m_upperLeft.ToTileId(zoomLevel);

    TileIdList tileids;
    // Get tileids of all tiles in the rectangular area
    for (auto row = ultileid.m_row; row <= lrtileid.m_row; ++row)
        {
        for (auto col = ultileid.m_column; col <= lrtileid.m_column; ++col)
            {
            TileId tid(zoomLevel, col, row);
            tileids.push_back(tid);
            }
        }

    DEBUG_PRINTF("Tile count=%d", tileids.size());
    return tileids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawFinerTiles(DrawArgs& args, TileId tileid, uint32_t maxLevelsToTry)
    {
    if (tileid.m_zoomLevel >= MAX_ZOOM_LEVEL)
        return;

    TileId finerUlTileid(tileid); // The tile in the upper left corner of the tile that we need to fill.

    ++finerUlTileid.m_zoomLevel;
    finerUlTileid.m_row <<= 1;
    finerUlTileid.m_column <<= 1;

    // Get tiles at the next finer resolution to fill in the space defined by the specified tileid
    for (uint32_t row=0; row < 2; ++row)
        {
        for (uint32_t col=0; col < 2; ++col)
            {
            TileId finerTileid(finerUlTileid);
            finerTileid.m_column += col;
            finerTileid.m_row += row;

            auto finerTile = GetLoadedTile(finerTileid);
            if (finerTile.IsValid())
                args.DrawTile(*finerTile);
            else
                {
                if (maxLevelsToTry > 1)
                    DrawFinerTiles(args, finerTileid, maxLevelsToTry-1);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawCoarserTiles(DrawArgs& args, uint8_t zoomLevel, uint32_t maxLevelsToTry)
    {
    TileIdList tileIds = GetTileIds(zoomLevel);

    bool allFound = true;
    for (auto const& tileid  : tileIds)
        {
        auto tile = GetLoadedTile(tileid);
        if (!tile.IsValid())
            allFound = false;
        else
            args.DrawTile(*tile);
        }

    args.DrawGraphics(-.01);

    if (!allFound && maxLevelsToTry>0 && zoomLevel>0)
        DrawCoarserTiles(args, zoomLevel-1, maxLevelsToTry-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    WebMercatorDisplay display(*this, *context.GetViewport());
    display.DrawView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawArgs::DrawGraphics(double bias)
    {
    if (m_graphics.m_entries.empty())
        return;

    DEBUG_PRINTF("WebMercator drawing %d tiles", m_graphics.m_entries.size());
                       
    Transform biasTrans;
    biasTrans.InitFrom(DPoint3d::From(0.0, 0.0, bias));
    auto group = m_context.CreateGroupNode(Graphic::CreateParams(nullptr, biasTrans), m_graphics, nullptr);
    m_context.OutputGraphic(*group, nullptr);

    BeAssert(m_graphics.m_entries.empty()); // the CreateGroupNode should have moved them
    }

/*---------------------------------------------------------------------------------**//**
* This method must be fast. Only access loaded tiles.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawView(TerrainContextR context)
    {
    DgnDb::VerifyClientThread();
    if (!IsValid())
        return;

    //  Decide what zoom level we should use for a view of this size.
    if (!ComputeZoomLevel(*context.GetViewport()))
        return; // the xy plane is edge-on in view

    TileIdList tileIds = GetTileIds(m_zoomLevel);
    if (tileIds.empty())
        return;

    Render::SystemR renderSys = context.GetTargetR().GetSystem();
    DrawArgs args(context);

    for (auto const& tileId  : tileIds)
        {
        auto tile = GetCachedTile(tileId);
        if (!tile.IsValid())
            tile = m_model.CreateTile(tileId, ComputeCorners(tileId), renderSys);

        if (tile->IsLoaded())
            args.DrawTile(*tile);
        else
            args.AddMissingTile(*tile);
        }

    args.DrawGraphics(0.0);

    if (args.m_missing.empty())
        return;

    for (auto const& missing : args.m_missing)
        {
        DrawFinerTiles(args, missing->GetTileId(), 2);
        args.DrawGraphics(-.001);
        }

    if (m_zoomLevel > 0)
        {
        //  If we are missing any tiles, first draw the whole view at a coarser zoomLevel. That provides a backdrop. We'll fill in the tiles that we do have after this.
        //  We draw the whole view rather than substitute for individual missing tiles because we can use the texture cache to draw the whole view.
        DrawCoarserTiles(args, m_zoomLevel - 1, 3);
        }

    // register for progressive display.
    DEBUG_PRINTF("%d missing tiles", args.m_missing.size());
    context.GetViewport()->ScheduleTerrainProgressiveTask(*new WebMercatorProgressive(args.m_missing));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ProgressiveTask::Completion WebMercatorProgressive::_DoProgressive(ProgressiveContext& context, WantShow& wantShow)
    {
    uint64_t now = BeTimeUtilities::QueryMillisecondsCounter();
    DrawArgs args(context);

    DEBUG_PRINTF("webmercator progressive %d missing", m_missingTiles.size());

    for (auto const& tile: m_missingTiles)
        {
        if (tile->IsLoaded())
            args.DrawTile(*tile);
        else if (tile->IsQueued())
            args.AddMissingTile(*tile);     // still not ready, put into new missing list
        }

    args.DrawGraphics(0.0);  // the tiles that newly arrived drew into the GraphicsArray in the DrawArgs. Add them to the context in a GroupNode

    m_missingTiles.swap(args.m_missing); // swap the list of missing tiles we were waiting for with those that are still missing.

    DEBUG_PRINTF("webmercator still %d missing", m_missingTiles.size());

    if (m_missingTiles.empty()) // when we have no missing tiles, the progressive task is done. 
        {
        context.GetViewport()->SetNeedsHeal(); // unfortunately the newly drawn tiles may be obscured by lower resolution ones due to transparency
        return Completion::Finished;
        }

    if (now > m_nextShow)
        {
        m_nextShow = now + 1000;
        wantShow = WantShow::Yes;
        }

    return Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorDisplay::WebMercatorDisplay(WebMercatorModel const& model, DgnViewportR vp) : m_units(vp.GetViewController().GetDgnDb().Units()), m_model(model)
    {
    LatLongPoint centerLatLng = ConvertMetersToLatLng(DPoint3d::FromSumOf(*vp.GetViewOrigin(), *vp.GetViewDelta(), 0.5));
    m_originLatitudeInRadians = Angle::DegreesToRadians(centerLatLng.latitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::CreateOsmUrl(TileId tileid) const
{
    Utf8String url;

    if (!m_properties.m_mapType.empty() && m_properties.m_mapType[0] == '0')  // "(c) OpenStreetMap contributors"
        url = Utf8PrintfString("http://a.tile.openstreetmap.org/%d/%d/%d.png",tileid.m_zoomLevel, tileid.m_column, tileid.m_row);
    else // *** For now, use MapQuest for satellite images (just in developer builds) ***
        url = Utf8PrintfString("http://otile1.mqcdn.com/tiles/1.0.0/sat/%d/%d/%d.jpg",tileid.m_zoomLevel, tileid.m_column, tileid.m_row);  // "Portions Courtesy NASA/JPL-Caltech and U.S. Depart. of Agriculture, Farm Service Agency"

    // *** WIP_WEBMERCATOR m_features

    return url;
    }

#define MAPBOX_ACCESS_KEY "pk.eyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0.f7c9GAxz6j10kZvL_2DBHg"
#define MAPBOX_ACCESS_KEY_URI_ENCODED "pk%2EeyJ1IjoibWFwYm94YmVudGxleSIsImEiOiJjaWZvN2xpcW00ZWN2czZrcXdreGg2eTJ0In0%2Ef7c9GAxz6j10kZvL%5F2DBHg"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapModel::CreateMapBoxUrl(TileId tileid) const
    {
    Utf8String url;

    /*
    @2x.png     2x scale(retina)
    png32       32 color indexed PNG
    png64       64 color indexed PNG
    png128      128 color indexed PNG
    png256      256 color indexed PNG
    jpg70       70 % quality JPG
    jpg80       80 % quality JPG
    jpg90       90 % quality JPG
    */
    Utf8CP format = "png32";
    Utf8CP mapid = (!m_properties.m_mapType.empty() && m_properties.m_mapType [0] == '0')? "mapbox.streets": "mapbox.satellite";

    //                                                  m  z  x  y  f
    url = Utf8PrintfString("http://api.mapbox.com/v4/%s/%d/%d/%d.%s?access_token=", mapid, tileid.m_zoomLevel, tileid.m_column, tileid.m_row, format);
    url += MAPBOX_ACCESS_KEY_URI_ENCODED; // NB: This URI-encoded string must not be included in the sprintf format string!

    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool StreetMapModel::_ShouldRejectTile(TileId tileid, Utf8StringCR url, ByteStream const& data) const
    {
    if (m_properties.m_mapService[0] != '0' || m_properties.m_mapType[0] != '1')
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
    if (data.GetSize() == sizeof(s_mapbox_x) && 0==memcmp(data.GetData(), s_mapbox_x, data.GetSize()))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StreetMapModel::_CreateUrl(Utf8StringR url, TileId tileid) const
    {
    // The usual image format info
    if (m_properties.m_mapService.empty())
        {
        BeAssert(false && "missing map service");
        LOG.error("missing map service");
        return BSIERROR;
        }

    if (m_properties.m_mapService[0] == '0')
        {
        url = CreateMapBoxUrl(tileid);
        //printf ("url=%s\n", url.c_str());
        }
    else
        {
        BeAssert(false && "unrecognized map service");
        LOG.errorv("[%s] is an unrecognized map service", m_properties.m_mapService.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StreetMapModel::_GetCopyrightMessage() const
    {
    return m_properties.m_mapService[0] == '0' ? "(c) Mapbox, (c) OpenStreetMap contributors" : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getStreetMapServerDescription(StreetMapHandler::MapService mapService, StreetMapHandler::MapType mapType)
    {
    Utf8String descr;
    switch (mapService)
        {
        case StreetMapHandler::MapService::MapBox:
            {
            descr = ("Mapbox");   // *** WIP translate
            if (StreetMapHandler::MapType::Map == mapType)
                descr.append(" Map");   // *** WIP translate
            else
                descr.append(" Satellite Images"); // *** WIP translate
            break;
            }

#ifndef NDEBUG
        case StreetMapHandler::MapService::OpenStreetMaps:
            {
            descr = ("Open Street Maps");   // *** WIP translate
            if (StreetMapHandler::MapType::Map == mapType)
                descr.append(" Map");   // *** WIP translate
            else
                descr.append(" Satellite Images"); // *** WIP translate
            break;
            }
#endif
        }
    return descr;
    }

DEFINE_REF_COUNTED_PTR(WebMercatorModel)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId StreetMapHandler::CreateStreetMapModel(DgnDbR db, MapService mapService, MapType mapType, bool finerResolution)
    {
    //Utf8PrintfString modelName("com.bentley.dgn.StreetMap_%d_%d", mapService, mapType); // *** WIP_STREET_MAP how to make sure name is unique?
    // ANSWER to WIP: create a DgnAuthority and use the namespace
    Utf8String modelName = getStreetMapServerDescription(mapService,mapType).c_str();
    DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "StreetMapModel"));
    BeAssert(classId.IsValid());

    WebMercatorModelPtr model = new StreetMapModel(DgnModel::CreateParams(db, classId, DgnModel::CreateModelCode(modelName)));

    WebMercatorModel::Properties props;
    props.m_mapService = Utf8PrintfString("%d", mapService);
    props.m_mapType = Utf8PrintfString("%d", mapType);
    props.m_finerResolution = finerResolution;

    model->SetProperties(props);
    model->Insert();
    return model->GetModelId();
    }

static Utf8CP JSON_WebMercatorModel = "WebMercatorModel";
static Utf8CP PROPERTYJSON_MapService = "mapService";
static Utf8CP PROPERTYJSON_MapType = "mapType";
static Utf8CP PROPERTYJSON_FinerResolution = "finerResolution";
static Utf8CP PROPERTYJSON_GroundBias = "groundBias";
static Utf8CP PROPERTYJSON_Transparency = "transparency";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::ToJson(Json::Value& value) const
    {
    value[PROPERTYJSON_MapService] = m_mapService.c_str();
    value[PROPERTYJSON_MapType] = m_mapType.c_str();
    value[PROPERTYJSON_FinerResolution] = m_finerResolution;
    value[PROPERTYJSON_GroundBias] = m_groundBias;
    value[PROPERTYJSON_Transparency] = m_transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::Properties::FromJson(Json::Value const& value)
    {
    m_mapService = value[PROPERTYJSON_MapService].asString();
    m_mapType = value[PROPERTYJSON_MapType].asString();
    m_finerResolution = value[PROPERTYJSON_FinerResolution].asBool();
    m_groundBias = value.isMember(PROPERTYJSON_GroundBias) ? value[PROPERTYJSON_GroundBias].asDouble() : -1.0;
    m_transparency = value.isMember(PROPERTYJSON_Transparency) ? value[PROPERTYJSON_Transparency].asDouble() : 0.0;

    LIMIT_RANGE(0.0, .9, m_transparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_WriteJsonProperties(Json::Value& val) const
    {
    m_properties.ToJson(val[JSON_WebMercatorModel]);
    T_Super::_WriteJsonProperties(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_ReadJsonProperties(Json::Value const& val)
    {
    BeAssert(val.isMember(JSON_WebMercatorModel));
    m_properties.FromJson(val[JSON_WebMercatorModel]);
    T_Super::_ReadJsonProperties(val);
    }

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TiledRasterCache : BeSQLiteRealityDataStorage
{
    uint64_t m_allowedSize = MAX_DB_CACHE_SIZE;
    using BeSQLiteRealityDataStorage::BeSQLiteRealityDataStorage;
    mutable BeAtomic<bool> m_isPrepared;
    virtual bool _IsPrepared() const override {return m_isPrepared;}
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override;
    virtual BentleyStatus _CleanupDatabase(BeSQLite::Db& db) const override;
};

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct TileData : IRealityData<TileData, BeSQLiteRealityDataStorage, HttpRealityDataSource>
{
private:
    Utf8String  m_url;
    ByteStream  m_data;
    DateTime    m_creationDate;
    RgbImageInfo m_imageInfo;
    Utf8String  m_contentType;
    TilePtr     m_tile;
    ColorDef    m_color;
    Render::SystemR m_renderSys;

private:
    Utf8String SerializeRasterInfo() const;
    void DeserializeRasterInfo(Utf8CP);
    void LoadTile();

protected:
    virtual Utf8CP _GetId() const override {return m_url.c_str();}
    virtual bool _IsExpired() const override {return false;}
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body) override;
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key) override;
    virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const override;

public:
    TileData(TileR tile, Render::SystemR renderSys, ColorDef color) : m_tile(&tile), m_renderSys(renderSys), m_color(color) {}
    ByteStream const& GetData() const {return m_data;}
    DateTime GetCreationDate() const {return m_creationDate;}
    Utf8String const& GetContentType() const {return m_contentType;}
};
DEFINE_REF_COUNTED_PTR(TileData)

END_UNNAMED_NAMESPACE


#define TABLE_NAME_TiledRaster  "TiledRaster"
/*-----------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis           03/2015
+---------------+---------------+---------------+---------------+---------------+--*/
BentleyStatus TiledRasterCache::_PrepareDatabase(BeSQLite::Db& db) const
    {
    if (db.TableExists(TABLE_NAME_TiledRaster))
        {
        m_isPrepared.store(true);
        return SUCCESS;
        }

    Utf8CP ddl = "Url CHAR PRIMARY KEY,Raster BLOB,RasterSize INT,RasterInfo CHAR,ContentType CHAR,Created BIGINT,Expires BIGINT,ETag CHAR";
    if (BeSQLite::BE_SQLITE_OK == db.CreateTable(TABLE_NAME_TiledRaster, ddl))
        {
        m_isPrepared.store(true);
        return SUCCESS;
        }
    return ERROR;
    }

/*-----------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis           03/2015
+---------------+---------------+---------------+---------------+---------------+--*/
BentleyStatus TiledRasterCache::_CleanupDatabase(BeSQLite::Db& db) const
    {
    CachedStatementPtr sumStatement;
    db.GetCachedStatement(sumStatement, "SELECT SUM(RasterSize) FROM " TABLE_NAME_TiledRaster);

    if (BE_SQLITE_ROW != sumStatement->Step())
        return ERROR;

    uint64_t sum = sumStatement->GetValueInt64(0);
    if (sum <= m_allowedSize)
        return SUCCESS;

    uint64_t garbageSize = sum - m_allowedSize;

    CachedStatementPtr selectStatement;
    db.GetCachedStatement(selectStatement, "SELECT RasterSize,Created FROM " TABLE_NAME_TiledRaster " ORDER BY Created ASC");

    uint64_t runningSum=0;
    while (runningSum < garbageSize)
        {
        if (BE_SQLITE_ROW != selectStatement->Step())
            {
            BeAssert(false);
            return ERROR;
            }

        runningSum += selectStatement->GetValueInt64(0);
        }

    BeAssert (runningSum >= garbageSize);
    uint64_t creationDate = selectStatement->GetValueInt64(1);
    BeAssert (creationDate > 0);

    CachedStatementPtr deleteStatement;
    db.GetCachedStatement(deleteStatement, "DELETE FROM " TABLE_NAME_TiledRaster " WHERE Created <= ?");
    deleteStatement->BindInt64(1, creationDate);

    return BE_SQLITE_DONE == deleteStatement->Step() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void TileData::LoadTile()
    {
    Render::Image rgba;
    BentleyStatus readStatus = ERROR;
    if (m_contentType.Equals("image/png"))
        {
        readStatus = m_imageInfo.ReadImageFromPngBuffer(rgba, m_data.GetData(), m_data.GetSize());
        }
    else if (m_contentType.Equals("image/jpeg"))
        {
        readStatus = m_imageInfo.ReadImageFromJpgBuffer(rgba, m_data.GetData(), m_data.GetSize());
        }

    if (ERROR == readStatus || !rgba.IsValid())
        {
        m_tile->SetNotFound();
        return;
        }

    BeAssert(!m_imageInfo.m_isBGR);
    auto graphic = m_renderSys._CreateGraphic(Graphic::CreateParams(nullptr));
    auto texture = m_renderSys._CreateImageTexture(rgba, false);

    graphic->SetSymbology(m_color, m_color, 0);
    graphic->AddTile(*texture, m_tile->m_corners.m_pts);
    graphic->Close();
    m_tile->m_graphic = graphic;
    m_tile->SetLoaded();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::_InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, ByteStream const& body)
    {
    m_url.AssignOrClear(url);
    m_creationDate = DateTime::GetCurrentTime();

    auto contentTypeIter = header.find("Content-Type");
    if (contentTypeIter == header.end())
        return ERROR;

    m_contentType = contentTypeIter->second.c_str();
    m_data = body;

    LoadTile();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::_InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key)
    {
    BeMutexHolder lock(cs);

    CachedStatementPtr stmt;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Raster,RasterSize,RasterInfo,Created,Expires,ETag,ContentType FROM " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    stmt->ClearBindings();
    stmt->BindText(1, key, BeSQLite::Statement::MakeCopy::Yes);

    if (BeSQLite::BE_SQLITE_ROW != stmt->Step())
        {
#ifdef DEBUG_MERCATOR
        Utf8String tmp(key+40, 16);
        DEBUG_PRINTF("missing %s", tmp.c_str());
#endif
        return ERROR;
        }

    m_url = key;
    auto raster     = stmt->GetValueBlob(0);
    auto rasterSize = stmt->GetValueInt(1);
    m_data.SaveData((Byte*) raster, rasterSize);

    DeserializeRasterInfo(stmt->GetValueText(2));
    DateTime::FromUnixMilliseconds(m_creationDate,(uint64_t) stmt->GetValueInt64(3));
    m_contentType = stmt->GetValueText(6);

    DateTime expirationDate;
    DateTime::FromUnixMilliseconds(expirationDate,(uint64_t) stmt->GetValueInt64(4));
    SetExpirationDate(expirationDate);
    SetEntityTag(stmt->GetValueText(5));

    LoadTile();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileData::_Persist(Db& db, BeMutex& cs) const
    {
    BeMutexHolder lock(cs);

#ifdef DEBUG_MERCATOR
    Utf8String tmp(GetId()+40, 16);
    DEBUG_PRINTF("arrived  %s", tmp.c_str());
#endif

    int bufferSize = (int) GetData().GetSize();

    int64_t creationTime = 0;
    if (SUCCESS != GetCreationDate().ToUnixMilliseconds(creationTime))
        return ERROR;

    int64_t expirationDate = 0;
    if (SUCCESS != GetExpirationDate().ToUnixMilliseconds(expirationDate))
        return ERROR;

    CachedStatementPtr selectStatement;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Url FROM " TABLE_NAME_TiledRaster " WHERE Url=?"))
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
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TiledRaster "(Url,Raster,RasterSize,RasterInfo,Created,Expires,ETag,ContentType) VALUES(?,?,?,?,?,?,?,?)"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindText(1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindBlob(2, GetData().GetData(), bufferSize, BeSQLite::Statement::MakeCopy::No);
        stmt->BindInt(3, bufferSize);
        stmt->BindText(4, SerializeRasterInfo().c_str(), BeSQLite::Statement::MakeCopy::Yes);
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
Utf8String TileData::SerializeRasterInfo() const
    {
    Json::Value json;
    json["hasAlpha"] = m_imageInfo.m_hasAlpha;
    json["height"] = m_imageInfo.m_height;
    json["width"] = m_imageInfo.m_width;
    json["isBGR"] = m_imageInfo.m_isBGR;
    json["isTopDown"] = m_imageInfo.IsTopDown();
    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void TileData::DeserializeRasterInfo(Utf8CP serializedJson)
    {
    Json::Value json;
    Json::Reader reader;
    reader.parse(serializedJson, json);

    m_imageInfo.m_hasAlpha = json["hasAlpha"].asBool();
    m_imageInfo.m_height = json["height"].asInt();
    m_imageInfo.m_width = json["width"].asInt();
    m_imageInfo.m_isBGR = json["isBGR"].asBool();
    m_imageInfo.SetBottomUp(!json["isTopDown"].asBool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RealityDataCache& WebMercatorModel::GetRealityDataCache() const
    {
    if (!m_realityDataCache.IsValid())
        {
        m_realityDataCache = new RealityDataCache();

        BeFileName storageFileName = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
        storageFileName.AppendToPath(BeFileName(GetName()));
        storageFileName.AppendExtension(L"tilecache");

        uint32_t threadCount = std::max((uint32_t) 2,BeThreadUtilities::GetHardwareConcurrency() / 2);
        m_realityDataCache->RegisterStorage(*new TiledRasterCache(storageFileName, SchedulingMethod::LIFO, 4));
        m_realityDataCache->RegisterSource(*HttpRealityDataSource::Create(threadCount, SchedulingMethod::LIFO));
        }

    return *m_realityDataCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::RequestTile(TileId id, TileR tile, Render::SystemR sys) const
    {
    DgnDb::VerifyClientThread();

    tile.SetQueued();

    Utf8String url;
    _CreateUrl(url, id);

    ColorDef color = ColorDef::White();
    if (0.0 != m_properties.m_transparency)
        color.SetAlpha((Byte) (255.* m_properties.m_transparency));

    TileDataPtr data = new TileData(tile, sys, color);
    GetRealityDataCache().Get(*data, url.c_str(), RealityDataOptions());
    }
