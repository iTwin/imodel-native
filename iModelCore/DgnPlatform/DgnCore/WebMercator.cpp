/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/ImageUtilities.h>

#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

DPILOG_DEFINE(WebMercator)
#define LOG(sev,...) {if (WebMercator_getLogger().isSeverityEnabled(sev)) { WebMercator_getLogger().messagev (sev, __VA_ARGS__); }}

static bool s_drawSubstitutes = true;

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
namespace {
static int const TILE_SIZE = 256;

static Upoint2d s_pixelOrigin = {TILE_SIZE / 2, TILE_SIZE / 2};
static double s_pixelsPerLonDegree = TILE_SIZE / 360.0;
static double s_pixelsPerLonRadian = TILE_SIZE / msGeomConst_2pi;

static double s_minLatitude  =  -85.05112878;
static double s_maxLatitude  =   85.05112878;
static double s_minLongitude = -180.0;
static double s_maxLongitude =  180.0;
//static double s_equatorialCircumference = 6372.7982;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static double bound(double value, double opt_min, double opt_max) 
    {
    value = std::max(value, opt_min);
    value = std::min(value, opt_max);
    return value;
    }

#ifdef WEBMERCATOR_DEBUG_TILES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct DMS
    {
    Int32 degrees, minutes;
    double seconds;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DMS decToSexagesimal (double degrees)
    {
    Int32 sign = 1;
    if (degrees < 0)
        {
        sign = -1;
        degrees = fabs(degrees);
        }

    DMS dms;
    dms.degrees = (Int32) floor(degrees);      // the whole part is degrees
    auto mins = (degrees-dms.degrees)*60.0;    // the remainder is minutes
    dms.minutes = (Int32) floor(mins);         // the whole part of that is minutes
    dms.seconds = (mins - dms.minutes)*60.0;   // the remainder is seconds
    dms.degrees *= sign;
    return dms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String fmtSexagesimal (DMS const& dms)
    {
    return Utf8PrintfString ("%d:%d:%0.2lf", dms.degrees, dms.minutes, dms.seconds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String fmtSexagesimal (GeoPointCR gp)
    {
    return fmtSexagesimal(decToSexagesimal(gp.latitude)) + "\n" + fmtSexagesimal(decToSexagesimal(gp.longitude));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WEBMERCATOR_DEBUG_TILES
static void setSymbology (ViewContextR context, RgbColorDefCR color, UInt32 trans, UInt32 width)
    {
    ElemMatSymb elemMatSymb;

    auto colorIdx = context.GetViewport()->MakeTrgbColor(color.red, color.green, color.blue, trans);
    elemMatSymb.SetLineColorTBGR (colorIdx);
    elemMatSymb.SetWidth (width);

    context.GetIDrawGeom().ActivateMatSymb (&elemMatSymb);
    }
#endif


#ifndef NDEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isZoomLevelInRange (UInt8 zoomLevel)
    {
    return zoomLevel <= WebMercatorTilingSystem::MAX_ZOOM_LEVEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isGeoPointInRange (GeoPointCR latLng)
    {
    return (s_minLatitude  <= latLng.latitude   && latLng.latitude  <= s_maxLatitude
        &&  s_minLongitude <= latLng.longitude  && latLng.longitude <= s_maxLongitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isWpixelPointInRange (DPoint2dCR point)
    {
    return (0 <= point.x && point.x <= TILE_SIZE) && (0 <= point.y && point.y <= TILE_SIZE);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void clampGeoPoint (GeoPointR latLng)
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
static void clampWpixelCoordinate (double& v)
    {
    if (v < 0)
        v = 0;
    if (BeNumerical::Compare (v, TILE_SIZE) >= 0)
        v = (TILE_SIZE - 1.0e-9);   // fudge the greatest possible coordinate as being just a little inside the edge.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static void clampWpixelPoint (DPoint2dR point)
    {
    clampWpixelCoordinate (point.x);
    clampWpixelCoordinate (point.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d fromGeoToWpixelPoint (GeoPointCR latLng) 
    {
    BeAssert (isGeoPointInRange(latLng));

    DPoint2d point;
        
    point.x = s_pixelOrigin.x + latLng.longitude * s_pixelsPerLonDegree;

    // Truncating to 0.9999 effectively limits latitude to 89.189. This is
    // about a third of a tile past the edge of the world tile.
    auto siny = bound(sin(Angle::DegreesToRadians(latLng.latitude)), -0.9999, 0.9999);
    point.y = s_pixelOrigin.y + 0.5 * log((1 + siny) / (1 - siny)) * -s_pixelsPerLonRadian;

    clampWpixelPoint (point);

    return point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
GeoPoint fromWpixelToGeoPoint (DPoint2dCR point) 
    {
    BeAssert (isWpixelPointInRange (point));

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
DPoint2d fromWpixelToPixelPoint (DPoint2dCR wpixels, UInt8 zoomLevel)
    {
    BeAssert (isZoomLevelInRange (zoomLevel));
    BeAssert (isWpixelPointInRange (wpixels));
    
    UInt32 numTiles = 1 << zoomLevel;
    return DPoint2d::From (wpixels.x * numTiles, wpixels.y * numTiles);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Upoint2d fromPixelPointToTileCoordinates (DPoint2dCR pixelPoint)
    {
    Upoint2d tileid;
    tileid.x = (decltype(tileid.x)) floor (pixelPoint.x / TILE_SIZE);
    tileid.y = (decltype(tileid.y)) floor (pixelPoint.y / TILE_SIZE);
    return tileid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorTilingSystem::TileId fromWpixelPointToTileId (DPoint2dCR wpixelPoint, UInt8 zoomLevel)
    {
    BeAssert (isZoomLevelInRange (zoomLevel));
    Upoint2d xy = fromPixelPointToTileCoordinates (fromWpixelToPixelPoint (wpixelPoint, zoomLevel));
    WebMercatorTilingSystem::TileId tileid;
    tileid.column = xy.x;
    tileid.row    = xy.y;
    tileid.zoomLevel =  zoomLevel;
    return tileid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d fromTileIdToWpixelPoint (WebMercatorTilingSystem::TileId const& tileid)
    {
    auto pix = tileid.column * TILE_SIZE;
    auto piy = tileid.row    * TILE_SIZE;

    DPoint2d wpixelPoint;
    UInt32 numTiles = 1 << tileid.zoomLevel;
    wpixelPoint.x = pix / (double)numTiles;
    wpixelPoint.y = piy / (double)numTiles;
    
    return wpixelPoint;
    }

/*
static void demo (UInt32 zoomLevel)
    {
    GeoPoint chicago {-87.6500523, 41.850033};

    auto wpixelPoint = fromGeoToWpixelPoint (chicago);
    auto pixelPoint = fromWpixelToPixelPoint (wpixelPoint, zoomLevel);
    auto tileCoordinates = fromPixelPointToTileCoordinates (pixelPoint);

    std::stringstream ss;
    ss <<
      "Chicago, IL" <<
      "GeoPoint: " << chicago.latitude << " , " << chicago.longitude << "\n" <<
      "World Coordinate: " << wpixelPoint.x << " , " <<  wpixelPoint.y << "\n" <<
      "Pixel Coordinate: " << floor(pixelPoint.x) << " , " << floor(pixelPoint.y) << "\n" <<
      "Tile Coordinate: " << tileCoordinates.x << " , " << tileCoordinates.y << " at Zoom Level: " << zoomLevel;

    printf ("%s\n", ss.str().c_str());
    }
    */

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static double computeGroundResolutionInMeters (UInt8 zoomLevel, double latitude)
    {
    // "Exact length of the equator (according to wikipedia) is 40075.016686 km in WGS-84. A horizontal tile size at zoom 0 would be 156543.034 meters" (http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale)
    return 156543.034 * cos(latitude) / (1<<zoomLevel);
    }

}; // anonymous namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef GRAPHITE0502
#define GET_METERS_PER_UOR(units) units.GetPhysicalUnits().ConvertFromUorsToMeters();
#else
// In Graphite06, data is stored in mm. Therefore there is 1/1000 of a meter per mm.
#define GET_METERS_PER_UOR(units) 0.001
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double WebMercatorUorConverter::ComputeViewResolutionInMetersPerPixel (ViewportR vp)
    {
    DRange3d    range;
    vp.GetViewCorners (range.low, range.high); // lower left back, upper right front    -- View coordinates aka "pixels"
    
    DPoint3d corners[8];
    range.Get8Corners (corners);    // pixels

    double viewDiagInPixels = corners[0].Distance (corners[3]); // pixels

    DPoint3d cornersWorld[8];
    vp.ViewToWorld (cornersWorld, corners, _countof(cornersWorld));

    double viewDiagInUors = cornersWorld[0].Distance (cornersWorld[3]); // UORs
    
    auto viewDiagInMeters = viewDiagInUors * m_meters_per_uor;    // meters
    
    return viewDiagInMeters / viewDiagInPixels;                 // meters/pixel
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus projectPointToPlaneInDirection (DPoint3d& pointOnPlane, DPlane3dCR plane, DPoint3dCR pt, DVec3dCR dir)
    {
    DRay3d ray;
    ray.origin = pt;
    ray.direction = dir;
    double param;
    return ray.Intersect (pointOnPlane, param, plane)? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorUorConverter::WebMercatorUorConverter (ViewportR vp) : m_units(vp.GetViewController().GetDgnProject().Units())
    {
    GeoPoint centerLatLng = ConvertUorsToLatLng (DPoint3d::FromSumOf (*vp.GetViewOrigin(), *vp.GetViewDelta(), 0.5));
    m_originLatitudeInRadians = Angle::DegreesToRadians (centerLatLng.latitude);

    m_meters_per_uor = GET_METERS_PER_UOR(m_units);        // (meters/UOR)

    // NB: define m_originLatitudeInRadians before calling ComputeGroundResolutionInMeters
    }

/*---------------------------------------------------------------------------------**//**
* Compute meters/pixel at the Project's geo origin the specified zoomLevel.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
double WebMercatorUorConverter::ComputeGroundResolutionInMeters (UInt8 zoomLevel)
    {
    return computeGroundResolutionInMeters (zoomLevel, m_originLatitudeInRadians); // meters/pixel
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
GeoPoint WebMercatorUorConverter::ConvertUorsToLatLng (DPoint3dCR dp)
    {
    auto i = m_uorsToLatLng.find (dp);
    if (i != m_uorsToLatLng.end())
        return i->second;

    GeoPoint gp;
    m_units.LatLongFromUors (gp, dp);

    m_uorsToLatLng[dp] = gp;
    m_latLngToUors[gp] = dp;

    return gp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d WebMercatorUorConverter::ConvertLatLngToUors (GeoPoint gp)
    {
    auto i = m_latLngToUors.find (gp);
    if (i != m_latLngToUors.end())
        return i->second;

    DPoint3d dp;
    m_units.UorsFromLatLong(dp, gp);

    m_latLngToUors[gp] = dp;
    m_uorsToLatLng[dp] = gp;

    return dp;
    }

/*---------------------------------------------------------------------------------**//**
* Convert a DPoint2d expressed in the Project's UOR coordinate system to a DPoint2d expressed in wpixels.
* Note that this function can fail, if pt is outside the WebMercator box. In that case, 
* an out-of-range point is clamped to the edge of the box.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d WebMercatorUorConverter::ConvertUorsToWpixels (DPoint2dCR pt)
    {
    auto pt3 = DPoint3d::From (pt.x,pt.y,0);
    auto latlng = ConvertUorsToLatLng (pt3);
    clampGeoPoint (latlng);
    return fromGeoToWpixelPoint (latlng);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorTileDisplayHelper::WebMercatorTileDisplayHelper (ViewportR vp)
    :
    m_converter (vp),
    m_drawingSubstituteTiles (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorUorConverter::ComputeTileCorners (DPoint3d* corners, WebMercatorTilingSystem::TileId const& tileid)
    {
    WebMercatorTilingSystem::TileId t11 (tileid);
    t11.row = t11.column = 1;
    auto t11Wpixels = fromTileIdToWpixelPoint (t11);
    auto tileSizeWpixels = t11Wpixels.x;


    // We want 4 corners in the wpixel coordinate system that Google maps uses
    //    ----x----->
    //  | [0]     [1]
    //  y            
    //  | [2]     [3]
    //  v
    DPoint2d cornersWpixels[4];
    cornersWpixels[0] = fromTileIdToWpixelPoint (tileid);    // returns upper left corner of tile
    cornersWpixels[1].SumOf (cornersWpixels[0], DPoint2d::From (tileSizeWpixels,  0              ));
    cornersWpixels[2].SumOf (cornersWpixels[0], DPoint2d::From (0,                tileSizeWpixels));
    cornersWpixels[3].SumOf (cornersWpixels[0], DPoint2d::From (tileSizeWpixels,  tileSizeWpixels));

    for (size_t i=0; i<_countof(cornersWpixels); ++i)
        {
        corners[i] = ConvertLatLngToUors (fromWpixelToGeoPoint (cornersWpixels[i]));
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct TextureCache
    {
    struct Entry 
        {
        uintptr_t   m_textureId;
        ImageUtilities::RgbImageInfo m_imageInfo;
        UInt64      m_insertTime;
        };

    bmap<Utf8String, Entry> m_map;
    uintptr_t               m_nextTextureId;

    static TextureCache* s_instance;
    static TextureCache& Instance() { if (NULL==s_instance) s_instance = new TextureCache; return *s_instance;}

    TextureCache() : m_nextTextureId(0) {;}

    // Issue the next texture id that can be used to define a new texture. 
    // Must ensure that no existing texture has this ID!
    // When we trim, we leave some textures in the cache with lower number texture ids. All new textures must be greater than those ids.
    uintptr_t GetNextTextureId() 
        {
        auto tid = (uintptr_t)this + m_nextTextureId++;
        #ifndef NDEBUG
            for (auto const& e : m_map)
                {
                BeAssert (e.second.m_textureId != tid);
                }
        #endif
        return tid;
        }  

    void Insert (Utf8StringCR url, ImageUtilities::RgbImageInfo const& info, uintptr_t tid)
        {
        auto& entry = m_map[url];

        if (entry.m_textureId != 0)
            T_HOST.GetGraphicsAdmin()._DeleteTexture (entry.m_textureId);

        entry.m_textureId = tid;
        entry.m_imageInfo = info;
        entry.m_insertTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        }

    Entry const* Get (Utf8StringCR url)
        {
        auto ifound = m_map.find (url);
        return (ifound == m_map.end())? NULL: &ifound->second;
        }

    bool IsCacheTooLarge () const {return m_map.size() >= 200;}

    void Trim ()
        {
        bmap<UInt64, bvector<Utf8String>> lru;
        for (auto const& mapentry: m_map)
            {
            lru[mapentry.second.m_insertTime].push_back (mapentry.first);
            }

        for (auto const& lruentry : lru)
            {
            for (auto const& url : lruentry.second)
                {
                if (!IsCacheTooLarge ())
                    return;
                T_HOST.GetGraphicsAdmin()._DeleteTexture (m_map[url].m_textureId);
                m_map.erase (url);
                }
            }
        }
    };

TextureCache* TextureCache::s_instance;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorTileDisplayHelper::GetCachedTexture (uintptr_t& cachedTextureId, ImageUtilities::RgbImageInfo& cachedImageInfo, Utf8StringCR url)
    {
    auto existingTexture = TextureCache::Instance().Get (url);
    if (existingTexture == NULL)
        return BSIERROR;

    cachedTextureId = existingTexture->m_textureId;
    cachedImageInfo = existingTexture->m_imageInfo;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t WebMercatorTileDisplayHelper::DefineTexture (bvector<byte> const& rgbData, ImageUtilities::RgbImageInfo const& imageInfo)
    {
    BeAssert (!imageInfo.isBGR);
    int format      = imageInfo.hasAlpha? QV_BGRA_FORMAT: QV_BGR_FORMAT;
    int sizeofPixel = imageInfo.hasAlpha? 4: 3;
    int pitch       = imageInfo.width * sizeofPixel;

    Point2d sizeInPixels;
    sizeInPixels.x = imageInfo.width;
    sizeInPixels.y = imageInfo.height;
        
    uintptr_t textureId = TextureCache::Instance().GetNextTextureId();
    T_HOST.GetGraphicsAdmin()._DefineTile (textureId, "", sizeInPixels, false, format, pitch, &rgbData[0]);

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorTileDisplayHelper::CacheTexture (Utf8StringCR url, uintptr_t textureId, ImageUtilities::RgbImageInfo const& imageInfo)
    {
    TextureCache::Instance().Trim();
    TextureCache::Instance().Insert (url, imageInfo, textureId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorTileDisplayHelper::DrawTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, ImageUtilities::RgbImageInfo const& imageInfo, uintptr_t textureId)
    {
    // Get 4 corners in this order:
    //  [0]     [1]
    //             
    //  [2]     [3]
    DPoint3d uvPts[4];
    m_converter.ComputeTileCorners (uvPts, tileid);

    if (!imageInfo.isTopDown)
        {
        std::swap (uvPts[0], uvPts[2]);
        std::swap (uvPts[1], uvPts[3]);
        }

    if (!context.GetViewport()->GetViewController().GetTargetModel()->Is3d())
        {
        for (auto& pt : uvPts)
            pt.z = -Viewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority
        }

    #ifdef WEBMERCATOR_DEBUG_TILES
        RgbColorDef color = {0,100,0};
        double z = 5000;
        if (m_drawingSubstituteTiles)
            {
            color.green = color.red = 200;
            z = -Viewport::GetDisplayPriorityFrontPlane();
            }
        setSymbology (context, color, 128, 1);
        DrawTileAsBox (context, tileid, z, false);
    #endif

    context.GetIViewDraw().DrawMosaic (1,1, &textureId, uvPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorTileDisplayHelper::DrawAndCacheTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, Utf8StringCR url, IRealityData& realityData)
    {
    // [Grigas] it would be better if TiledRastRealityData had a function "Draw" which could handle tiled raster drawing - we wouldn't need a cast then 
    auto tiledRasterRealityData = dynamic_cast<TiledRaster*> (&realityData);

    auto const& data = tiledRasterRealityData->GetData();
    auto const& expectedImageInfo = tiledRasterRealityData->GetImageInfo();
    ImageUtilities::RgbImageInfo actualImageInfo = tiledRasterRealityData->GetImageInfo();

    Utf8String contentType = tiledRasterRealityData->GetContentType();
    BentleyStatus status;

    m_rgbBuffer.clear(); // reuse the same buffer, in order to minimize mallocs

    if (contentType.Equals ("image/png"))
        {
        status = ImageUtilities::ReadImageFromPngBuffer (m_rgbBuffer, actualImageInfo, data.data(), data.size());
        if (SUCCESS != status)
            LOG (LOG_WARNING, "Invalid png image data: %s", url.c_str());
        }
    else if (contentType.Equals ("image/jpeg"))
        {
        status = ImageUtilities::ReadImageFromJpgBuffer (m_rgbBuffer, actualImageInfo, data.data(), data.size(), expectedImageInfo);
        if (SUCCESS != status)
            LOG (LOG_WARNING, "Invalid jpeg image data: %s", url.c_str());
        }
    else
        {
        BeAssert (false && "Unsupported image type");
        LOG (LOG_WARNING, "Unsupported image type: %s -> %s", url.c_str(), contentType.c_str());
        status = BSIERROR;
        }

    if (SUCCESS != status)
        return;

    auto tid = DefineTexture (m_rgbBuffer, actualImageInfo);
    CacheTexture (url, tid, actualImageInfo);
    DrawTile (context, tileid, actualImageInfo, tid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WEBMERCATOR_DEBUG_TILES
void WebMercatorTileDisplayHelper::DrawTileAsBox (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, double z, bool filled)
    {
    DPoint3d uvPts[4];
    m_converter.ComputeTileCorners (uvPts, tileid);

    for (auto& pt: uvPts)
        pt.z = z;
    
    DPoint3d box[5];
    box[0] = uvPts[0];
    box[1] = uvPts[1];
    box[2] = uvPts[3];
    box[3] = uvPts[2];
    box[4] = box[0];
    context.GetIDrawGeom().DrawShape3d (_countof(box), box, filled, NULL);

    //DPoint3d diagonal[2];
    //diagonal[0] = uvPts[2];
    //diagonal[1] = uvPts[1];
    //context.GetIDrawGeom().DrawLineString3d (2, diagonal, NULL);
    //
    //diagonal[0] = uvPts[0];
    //diagonal[1] = uvPts[3];
    //context.GetIDrawGeom().DrawLineString3d (2, diagonal, NULL);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorTileDisplayHelper::DrawMissingTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid)
    {
#ifdef WEBMERCATOR_DEBUG_TILES
    RgbColorDef color = {100,100,100};
    setSymbology (context, color, 215, 0);
    DrawTileAsBox (context, tileid, -Viewport::GetDisplayPriorityFrontPlane(), false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
#ifdef WEBMERCATOR_DEBUG_TILES
static void drawPoint (ViewContextR context, DPoint3dCR pt, bool drawCrossHair=false)
    {
    auto pixels = context.GetPixelSizeAtPoint(NULL);

    DEllipse3d circle;
    auto z = DVec3d::From (0,0,1);
    circle.InitFromCenterNormalRadius (pt, z, 5*pixels);
    context.GetIDrawGeom().DrawArc3d (circle, true, true, NULL);

    if (drawCrossHair)
        {
        DPoint3d pts[2];
        pts[0].SumOf (pt, DVec3d::From(0,-100*pixels,0));
        pts[1].SumOf (pt, DVec3d::From(0, 100*pixels,0));
        context.GetIDrawGeom().DrawLineString3d (2, pts, NULL);

        pts[0].SumOf (pt, DVec3d::From(-100*pixels,0,0));
        pts[1].SumOf (pt, DVec3d::From(100*pixels,0,0));
        context.GetIDrawGeom().DrawLineString3d (2, pts, NULL);
        }
    }
#endif
+---------------+---------------+---------------+---------------+---------------+------*/

#ifdef WEBMERCATOR_DEBUG_TILES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawText (ViewContextR context, DPoint3dCR ptUl, Utf8StringCR strings, TextElementJustification just = TextElementJustification::LeftBaseline)
    {
    auto pixels = context.GetPixelSizeAtPoint(NULL);
    
    DPoint2d textScale;
    textScale.Init (10*pixels, 10*pixels);
    TextStringPropertiesPtr props = TextStringProperties::Create (DgnFontManager::GetDefaultTrueTypeFont(), NULL, textScale);
    props->SetJustification (just);

    DPoint3d pt (ptUl);
    size_t offset = 0;
    Utf8String str;
    while ((offset = strings.GetNextToken (str, "\n", offset)) != Utf8String::npos)
        {
        TextStringPtr textString = TextString::Create (WString(str.c_str(),BentleyCharEncoding::Utf8).c_str(), NULL, NULL, *props);
        BeAssert (textString.IsValid());

        textString->SetOriginFromUserOrigin (pt);

        context.GetIDrawGeom().DrawTextString (*textString);

        pt.y -= 17*pixels;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorTilingSystem::GetOptimalZoomLevelForView (UInt8& zoomLevel, ViewportR vp, WebMercatorUorConverter& converter, bool preferFinerResolution)
    {
    // Get the number of meters / screen pixel that we are showing in the view
    double viewResolution = converter.ComputeViewResolutionInMetersPerPixel (vp); 

    // Return the zoom level that has about the same "ground resolution", which is defined as meters / pixel.
    for (UInt8 i=0; i<=MAX_ZOOM_LEVEL; ++i)
        {
        double gr = converter.ComputeGroundResolutionInMeters (i);
        if (BeNumerical::Compare (gr, viewResolution) <= 0)
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
BentleyStatus WebMercatorTilingSystem::GetTileIdsForView (bvector<TileId>& tileids, UInt8 zoomLevel, ViewportR vp, WebMercatorUorConverter& converter)
    {
    auto const& vc = vp.GetViewController();

    // Get upper left and lower right of view in WebMercator "world coordinates" (i.e., pixels at zoomLevel 0)
    // NB: The origin of the WebMercator tiling system is in the upper left.
    DPoint2d ul, lr;
        {
        // Get view corners
        auto rect = vp.GetViewRect();

        auto range = DRange3d::NullRange();
        range.Extend (vp.ViewToWorld (DPoint3d::From (rect.Left(), rect.Top())));
        range.Extend (vp.ViewToWorld (DPoint3d::From (rect.Left(), rect.Bottom())));
        range.Extend (vp.ViewToWorld (DPoint3d::From (rect.Right(), rect.Bottom())));
        range.Extend (vp.ViewToWorld (DPoint3d::From (rect.Right(), rect.Top())));

        // Get the projection of the view corners onto the x-y plane
        auto viewDir = vc.GetZVector();
        auto xyplane = DPlane3d::FromOriginAndNormal (DPoint3d::FromZero(), DVec3d::From (0, 0, 1.0));
        DPoint3d llptxy, urptxy;
        if (projectPointToPlaneInDirection (llptxy, xyplane, range.low, viewDir) != BSISUCCESS
         || projectPointToPlaneInDirection (urptxy, xyplane, range.high, viewDir) != BSISUCCESS)
            {
            LOG (LOG_INFO, "The view is not looking at any part of the x-y plane");
            return BSIERROR;
            }

        // Restate as upper left and lower right
        DPoint2d ulptxy, lrptxy;
        ulptxy.x = std::min (llptxy.x, urptxy.x);
        lrptxy.x = std::max (llptxy.x, urptxy.x);
        ulptxy.y = std::max (llptxy.y, urptxy.y);
        lrptxy.y = std::min (llptxy.y, urptxy.y);

        ul = converter.ConvertUorsToWpixels (ulptxy);
        lr = converter.ConvertUorsToWpixels (lrptxy);
        }

    // Get tileids of upper left and lower right
    clampWpixelPoint (ul);
    clampWpixelPoint (lr);

    WebMercatorTilingSystem::TileId ultileid = fromWpixelPointToTileId (ul, zoomLevel);
    WebMercatorTilingSystem::TileId lrtileid = fromWpixelPointToTileId (lr, zoomLevel);

    // Get tileids of all tiles in the rectangular area
    for (auto row = ultileid.row; row <= lrtileid.row; ++row)
        {
        for (auto col = ultileid.column; col <= lrtileid.column; ++col)
            {
            WebMercatorTilingSystem::TileId tid;
            tid.zoomLevel = zoomLevel;
            tid.column = col;
            tid.row = row;
            tileids.push_back (tid);      
            }
        }    

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool shouldDraw (ViewContextR context)
    {
    switch (context.GetDrawPurpose())
        {
        case DrawPurpose::Hilite:
        case DrawPurpose::Unhilite:
        case DrawPurpose::ChangedPre:       // Erase, rely on Healing.
        case DrawPurpose::RestoredPre:      // Erase, rely on Healing.
        case DrawPurpose::RangeCalculation:
        case DrawPurpose::Pick:
        case DrawPurpose::Flash:
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
BentleyStatus WebMercatorRealityDataHandler::DrawSubstituteTilesFinerFromTextureCache (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, UInt32 maxLevelsToTry)
    {
    if (tileid.zoomLevel >= WebMercatorTilingSystem::MAX_ZOOM_LEVEL)
        return BSIERROR;

    // Get tiles at the next finer resolution to fill in the space defined by the specified tileid

    WebMercatorTilingSystem::TileId finerUlTileid (tileid); // The tile in the upper left corner of the tile that we need to fill.
    ++finerUlTileid.zoomLevel;
    finerUlTileid.row <<= 1;
    finerUlTileid.column <<= 1;

    for (UInt32 row=0; row < 2; ++row)
        {
        for (UInt32 col=0; col < 2; ++col)
            {
            WebMercatorTilingSystem::TileId finerTileid (finerUlTileid);
            finerTileid.column += col;
            finerTileid.row += row;

            Utf8String finerUrl;
            ImageUtilities::RgbImageInfo finerImageInfo;
            if (_CreateUrl (finerUrl, finerImageInfo, finerTileid) != BSISUCCESS)
                continue;

            // *** NB: Do not try to read from the RealityDataCache. This method is called from _DrawView, and it must be fast!

            uintptr_t finerImageTid;
            if (m_helper.GetCachedTexture (finerImageTid, finerImageInfo, finerUrl) == BSISUCCESS)
                {
                m_helper.DrawTile (context, finerTileid, finerImageInfo, finerImageTid);
                }
            else
                {
                if (maxLevelsToTry > 1)
                    DrawSubstituteTilesFinerFromTextureCache (context, finerTileid, maxLevelsToTry-1);
                }
            }
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorRealityDataHandler::DrawCoarserTilesForViewFromTextureCache (ViewContextR context, UInt8 zoomLevel, UInt32 maxLevelsToTry)
    {
    bvector<TileDisplayImageData> coarserTileDisplayImageData;
    bool allCoarserTilesInTextureCache;
    if (GetCachedTiles (coarserTileDisplayImageData, allCoarserTilesInTextureCache, zoomLevel, context) == BSISUCCESS && allCoarserTilesInTextureCache)
        {
        for (auto const& coarserTileddata : coarserTileDisplayImageData)
            m_helper.DrawTile (context, coarserTileddata.tileid, coarserTileddata.imageInfo, coarserTileddata.textureId);
        return BSISUCCESS;
        }
    
    if (maxLevelsToTry == 0 || zoomLevel == 0)
        return BSIERROR;
        
    return DrawCoarserTilesForViewFromTextureCache (context, zoomLevel-1, maxLevelsToTry-1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WEBMERCATOR_DEBUG_TILES
void WebMercatorTileDisplayHelper::DrawTileDebugInfo (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid)
    {
    DPoint3d corners[4];
    m_converter.ComputeTileCorners (corners, tileid); // corners[0] is upper left
    DPoint3d ulUors = corners[0];
    GeoPoint ulGP = m_converter.ConvertUorsToLatLng(ulUors);
    ulUors.z = 50000;

    //RgbColorDef tccolor = {0,255,0};
    //setSymbology (context, tccolor, 128, 2);
    //drawPoint (context, ulUors);

    RgbColorDef txtcolor = {10,10,10};
    setSymbology (context, txtcolor, 0, 1);
    drawText (context, ulUors, Utf8PrintfString("%0.15lf\n%0.15lf",ulGP.latitude,ulGP.longitude)/*fmtSexagesimal(ulGP)*/);

    auto centerPt = DPoint3d::FromInterpolate (corners[0], 0.5, corners[3]);
    centerPt.z = 5000;
    drawText (context, centerPt, Utf8PrintfString("%d:(%d,%d)", tileid.zoomLevel, tileid.row, tileid.column), TextElementJustification::CenterMiddle);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorRealityDataHandler::GetCachedTiles (bvector<TileDisplayImageData>& tileDisplayImageData, bool& allFoundInTextureCache, UInt8 zoomLevel, ViewContextR context)
    {
    allFoundInTextureCache = true;
    bvector<WebMercatorTilingSystem::TileId> tileIds;
    
    if (WebMercatorTilingSystem::GetTileIdsForView (tileIds, zoomLevel, *context.GetViewport(), m_helper.m_converter) != BSISUCCESS)
        return BSIERROR; // we get an error if the ground is not in the view at all. In that case, there are no tiles to draw.

    for (auto tileid  : tileIds)
        {
        TileDisplayImageData data;
        data.tileid = tileid;

        Utf8String url;
        if (_CreateUrl (url, data.imageInfo, data.tileid) != BSISUCCESS)
            return BSIERROR; // ?? when would this ever happen??

        if (m_helper.GetCachedTexture (data.textureId, data.imageInfo, url) != BSISUCCESS)
            {
            data.textureId = 0;
            allFoundInTextureCache = false;
            }

        tileDisplayImageData.push_back (data);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorRealityDataHandler::_DrawView (ViewContextR context)
    {
    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display


    // Preconditions
    BeAssert (m_missingTilesToBeRequested.empty() && "_DrawView should only be called on a freshly created handler instance");
    BeAssert (m_missingTilesPending.empty() && "_DrawView should only be called on a freshly created handler instance");

    //
    //  First, determine if we can draw map tiles at all.
    //
    if (!shouldDraw (context) || NULL == context.GetViewport())
        return;

    m_hadError = true;

    if (context.GetViewport()->IsCameraOn())    // *** TBD: Not sure if we can support tiled raster in a perspective view or not. 
        return;                                 // ***      I would at least have to figure out how to compute what portions of the earth are in the view.

    if (!m_helper.m_converter.IsValid())
        return;

    //
    //  Decide what zoom level we should use for a view of this size.
    //
    if (WebMercatorTilingSystem::GetOptimalZoomLevelForView (m_optimalZoomLevel, *context.GetViewport(), m_helper.m_converter, m_preferFinerResolution) != BSISUCCESS)
        return;

#ifdef WIP_REALITY_DATACACHE
    T_HOST.GetRealityDataAdmin().GetCache().CancelAll<TiledRaster> (NULL);
#endif

    //
    //  Figure out what tiles will be needed to cover the view.
    //
    bvector<TileDisplayImageData> allTilesToBeDisplayed;
    bool allTilesInTextureCache;
    if (GetCachedTiles (allTilesToBeDisplayed, allTilesInTextureCache, m_optimalZoomLevel, context) != BSISUCCESS)
        return;

    m_hadError = false;

    if (allTilesToBeDisplayed.empty())
        return;

    bool coarserTilesDrawn = false;
    if (!allTilesInTextureCache && m_optimalZoomLevel > 0)
        {
        //  If we are missing any tiles, first draw the whole view at a coarser zoomLevel. That provides a backdrop. We'll fill in the tiles that we do have after this.
        //  We draw the whole view rather than substitute for individual missing tiles because we can use the texture cache to draw the whole view.
        coarserTilesDrawn = (BSISUCCESS == DrawCoarserTilesForViewFromTextureCache (context, m_optimalZoomLevel - 1, 3));
        }

    //
    //  Display each tile, if available. NB: Look only in the texture cache! Do not try to read from RealityDataCache!
    //
    for (auto const& tileddata : allTilesToBeDisplayed)
        {
        auto tileid = tileddata.tileid;
        if (tileddata.textureId != 0)
            {
            m_helper.DrawTile (context, tileid, tileddata.imageInfo, tileddata.textureId);
            }
        else
            {
            //  The image is not immediately available. We'll check the RealityDataCache and possibly request a download during progressive display.
            m_missingTilesToBeRequested.push_back (tileid);

            if (!coarserTilesDrawn)
                m_helper.DrawMissingTile (context, tileid); // draw something in the space (mainly for debugging)

            // Try to draw substitutes at a finer zoomLevel, if we can find them in the texture cache.
            DrawSubstituteTilesFinerFromTextureCache (context, tileid, 2);
            }
        }

    //
    //  If any tiles were not available, register for progressive display.
    //
    if (!m_missingTilesToBeRequested.empty())
        {
        context.GetViewport()->ScheduleProgressiveDisplay (*this);
        m_waitTime = 100;
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;
        }
    }

/*---------------------------------------------------------------------------------**//**
* This callback is invoked on a timer during progressive display.
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
IProgressiveDisplay::Completion WebMercatorRealityDataHandler::_Process (ViewContextR context)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        LOG (LOG_TRACE, "Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        return Completion::Aborted;
        }

    if (m_hadError)
        return Completion::Failed;

    // First, see if the RealityDataCache has the missing tiles. If so, draw them. If not, request downloads.
    // NOTE: It might take more than one pass to do this for each missing tile.
    while (!m_missingTilesToBeRequested.empty())
        {
        auto tileid = m_missingTilesToBeRequested.back();
        m_missingTilesToBeRequested.pop_back();
        
        Utf8String url;
        ImageUtilities::RgbImageInfo expectedImageInfo;
        _CreateUrl (url, expectedImageInfo, tileid);
        auto realityData = T_HOST.GetRealityDataAdmin ().GetCache ().Get<TiledRaster> (url.c_str (), *TiledRaster::RequestOptions::Create (expectedImageInfo), RealityDataGetCachedOption(true));
        if (realityData.IsValid())
            {
            //  The image is available from the cache. Great! Draw it.
            m_helper.DrawAndCacheTile (context, tileid, url, *realityData);
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
        auto realityData = T_HOST.GetRealityDataAdmin ().GetCache ().GetCached<TiledRaster> (url.c_str(), RealityDataGetCachedOption(true));

        if (context.CheckStop())
            return Completion::Aborted;

        if (realityData.IsValid())
            {
            //  Yes, we now have the image. Draw it and remove it from the list of missing tiles.
            m_helper.DrawAndCacheTile (context, tileid, url, *realityData);
            iMissing = m_missingTilesPending.erase (iMissing); 
            }
        else
            {
            ++iMissing; // Leave it in m_missingTilesPending to be retried later.
            }
        }

    if (!m_missingTilesPending.empty())
        {
        ++m_failedAttempts;
        m_waitTime = (UInt64)(m_waitTime * 1.33);
        if (m_failedAttempts > 5 && (m_failedAttempts&1)==0)
            {
            LOG (LOG_INFO, "%d failures. Wait %lld millis\n", m_failedAttempts, m_waitTime);
            }
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }

    //  Don't report "Finished" unless all missing tiles have been found and displayed.
    return m_missingTilesPending.empty()? Completion::Finished: Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorRealityDataHandler::WebMercatorRealityDataHandler (ViewportR vp) 
    : 
    m_optimalZoomLevel(0), 
    m_hadError(false),
    m_preferFinerResolution(false),
    m_drawSubstitutes(s_drawSubstitutes),
    m_helper(vp),
    m_waitTime(0),
    m_nextRetryTime(0),
    m_failedAttempts(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
// <summary>
// Converts tile XY coordinates into a QuadKey at a specified level of detail.
// </summary>
// <param name="tileX">Tile X coordinate.</param>
// <param name="tileY">Tile Y coordinate.</param>
// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
// to 23 (highest detail).</param>
// <returns>A string containing the QuadKey.</returns>
// source: http://msdn.microsoft.com/en-us/library/bb259689.aspx
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String tileXYToQuadKey (int tileX, int tileY, int levelOfDetail)
{
    Utf8String quadKey;
    for (int i = levelOfDetail; i > 0; i--)
        {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
            {
            digit++;
            }
        if ((tileY & mask) != 0)
            {
            digit++;
            digit++;
            }
        quadKey.append (1, digit);
        }
    return quadKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getCulture ()
    {
    // *** WIP_WEBMERCATOR culture
    return "en-US";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getBingToken ()
    {
    // *** WIP_WEBMERCATOR Bing - api token - I got this from D:\topaz\source\Imagepp\all\gra\hrf\src\HRFVirtualEarthFile.cpp, 
    // ***  but I don't know if it's valid in a shipping product.
    return "AjdiY0PuqXOEdT0JWVjvXqC3nydpHgDEhLcUwXtnKUUH_BW5u3pV3-Zhk5Ez_mwt";;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapRealityDataHandler::CreateBingUrl (WebMercatorTilingSystem::TileId const& tileid)
    {
    Utf8String bingURL = "http://";

    Utf8String quadKey = tileXYToQuadKey (tileid.column, tileid.row, tileid.zoomLevel);
    char const sub_domain[][3] = {{"t0"}, {"t1"}, {"t2"}, {"t3"}};
    
    bingURL += sub_domain[quadKey[quadKey.size()-1] - '0'];
    bingURL += ".tiles.virtualearth.net/tiles/r";
    bingURL += quadKey;
    bingURL += ".jpeg?";
    bingURL += "g=2229";    // *** WIP_WEBMERCATOR Bing - don't know what the g= parameter means
    bingURL += "&mkt=";
    bingURL += getCulture();
    bingURL += "&token=";
    bingURL += getBingToken();

    // *** WIP_WEBMERCATOR m_mapType
    // *** WIP_WEBMERCATOR m_features

    return bingURL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapRealityDataHandler::CreateGoogleMapsUrl (WebMercatorTilingSystem::TileId const& tileid)
    {
    // *** WIP_WEBMERCATOR We must append an API key -- need to get one.
    // *** WIP_WEBMERCATOR m_mapType
    // *** WIP_WEBMERCATOR m_features
    return Utf8PrintfString ("http://mts0.googleapis.com/vt?lyrs=m&x=%d&y=%d&z=%d", tileid.column, tileid.row, tileid.zoomLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StreetMapRealityDataHandler::CreateMapquestUrl (WebMercatorTilingSystem::TileId const& tileid)
    {
    Utf8String url = "http://otile1.mqcdn.com/tiles/1.0.0/";
    if (m_mapType == MapType::Map)
        url += "map/";
    else
        url += "sat/";  // "Portions Courtesy NASA/JPL-Caltech and U.S. Depart. of Agriculture, Farm Service Agency"

    // Filename(url) format is /zoom/x/y.png
    // see http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
    url += Utf8PrintfString ("%d/%d/%d.jpg",tileid.zoomLevel, tileid.column, tileid.row);

    // *** WIP_WEBMERCATOR m_features

    return url;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StreetMapRealityDataHandler::_CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& expectedImageInfo, WebMercatorTilingSystem::TileId const& tileid)
    {
    // The usual image format info
    expectedImageInfo.height = expectedImageInfo.width = 256;
    expectedImageInfo.hasAlpha = false;
    expectedImageInfo.isBGR = false;
    expectedImageInfo.isTopDown = true;

#ifndef NDEBUG
    if (m_mapService == MapService::TEST_OpenStreetMaps)
        url = Utf8PrintfString ("http://tile.openstreetmap.org/%d/%d/%d.png", tileid.zoomLevel, tileid.column, tileid.row);
    else 
#endif
    if (m_mapService == MapService::MapQuest) // "(c) OpenStreetMap contributors"
        url = CreateMapquestUrl (tileid);
    else if (m_mapService == MapService::GoogleMaps)
        url = CreateGoogleMapsUrl (tileid);
    else if (m_mapService == MapService::BlackAndWhite)
        url = Utf8PrintfString ("http://a.tile.stamen.com/toner/%d/%d/%d.png", tileid.zoomLevel, tileid.column, tileid.row);
    else if (m_mapService == MapService::Bing)
        url = CreateBingUrl (tileid);
    else
        {
        BeAssert (false && "unrecognized map service");
        LOG (LOG_ERROR, "%d is an unrecognized map service", m_mapService);
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
StreetMapRealityDataHandler::StreetMapRealityDataHandler (ViewportR vp, MapService mapService) 
    : 
    WebMercatorRealityDataHandler (vp),
    m_mapService (mapService),
    m_mapType (MapType::Map)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void StreetMapRealityDataHandler::SetMapType (MapType i)
    {
    m_mapType = i;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void StreetMapRealityDataHandler::SetFeature (Feature f, Utf8StringCR params)
    {
    m_features[f] = params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void StreetMapRealityDataHandler::RemoveFeature (Feature f)
    {
    m_features.erase (f);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void StreetMapRealityDataHandler::SetPreferFinerResolution (bool v)
    {
    m_preferFinerResolution = v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void LatLongGridRealityDataHandler::_DrawView (ViewContextR context)
    {
    /*
    auto vp = *context.GetViewport();

    DRange3d    range;
    vp.GetViewCorners (range.low, range.high); // lower left back, upper right front    -- View coordinates aka "pixels"
    
    DPoint3d corners[8];
    range.Get8Corners (corners);    // pixels

    double viewDiagInPixels = corners[0].Distance (corners[3]); // pixels

    DPoint3d cornersWorld[8];
    vp.ViewToWorld (cornersWorld, corners, _countof(cornersWorld));

    double viewDiagInUors = cornersWorld[0].Distance (cornersWorld[3]); // UORs
    
    auto viewDiagInMeters = viewDiagInUors * m_meters_per_uor;    // meters
    
    return viewDiagInMeters / viewDiagInPixels;                 // meters/pixel
    */
    }

