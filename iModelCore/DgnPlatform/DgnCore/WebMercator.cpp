/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/WebMercator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/ImageUtilities.h>

#define TABLE_NAME_TiledRaster  "TiledRaster"
#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

DPILOG_DEFINE(WebMercator)

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
    int32_t degrees, minutes;
    double seconds;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
static DMS decToSexagesimal (double degrees)
    {
    int32_t sign = 1;
    if (degrees < 0)
        {
        sign = -1;
        degrees = fabs(degrees);
        }

    DMS dms;
    dms.degrees = (int32_t) floor(degrees);      // the whole part is degrees
    auto mins = (degrees-dms.degrees)*60.0;    // the remainder is minutes
    dms.minutes = (int32_t) floor(mins);         // the whole part of that is minutes
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
static void setSymbology (ViewContextR context, RgbColorDefCR color, uint32_t trans, uint32_t width)
    {
    ElemMatSymb elemMatSymb;

    auto colorIdx = ColorDef(color.red, color.green, color.blue, trans);
    elemMatSymb.SetLineColor (colorIdx);
    elemMatSymb.SetWidth (width);

    context.GetIDrawGeom().ActivateMatSymb (&elemMatSymb);
    }
#endif


#ifndef NDEBUG
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isZoomLevelInRange (uint8_t zoomLevel)
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
DPoint2d fromWpixelToPixelPoint (DPoint2dCR wpixels, uint8_t zoomLevel)
    {
    BeAssert (isZoomLevelInRange (zoomLevel));
    BeAssert (isWpixelPointInRange (wpixels));
    
    uint32_t numTiles = 1 << zoomLevel;
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
WebMercatorTilingSystem::TileId fromWpixelPointToTileId (DPoint2dCR wpixelPoint, uint8_t zoomLevel)
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
    uint32_t numTiles = 1 << tileid.zoomLevel;
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
static double computeGroundResolutionInMeters (uint8_t zoomLevel, double latitude)
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
// In Graphite06, data is stored in meters. 
#define GET_METERS_PER_UOR(units) 1.0
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double WebMercatorUorConverter::ComputeViewResolutionInMetersPerPixel (DgnViewportR vp)
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
WebMercatorUorConverter::WebMercatorUorConverter (DgnViewportR vp) : m_units(vp.GetViewController().GetDgnDb().Units())
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
double WebMercatorUorConverter::ComputeGroundResolutionInMeters (uint8_t zoomLevel)
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
WebMercatorTileDisplayHelper::WebMercatorTileDisplayHelper (DgnViewportR vp)
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
        uint64_t    m_insertTime;
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
        bmap<uint64_t, bvector<Utf8String>> lru;
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
uintptr_t WebMercatorTileDisplayHelper::DefineTexture (bvector<Byte> const& rgbData, ImageUtilities::RgbImageInfo const& imageInfo)
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
        auto extents = context.GetViewport()->GetViewController().GetProjectExtents();
        for (auto& pt : uvPts)
            pt.z = extents.low.z - 1;
        }

    #ifdef WEBMERCATOR_DEBUG_TILES
        RgbColorDef color = {0,100,0};
        double z = 5000;
        if (m_drawingSubstituteTiles)
            {
            color.green = color.red = 200;
            z = -DgnViewport::GetDisplayPriorityFrontPlane();
            }
        setSymbology (context, color, 128, 1);
        DrawTileAsBox (context, tileid, z, false);
    #endif

    context.GetIViewDraw().DrawMosaic (1,1, &textureId, uvPts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorTileDisplayHelper::DrawAndCacheTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, Utf8StringCR url, TiledRaster& realityData)
    {
    auto const& data = realityData.GetData();
    auto const& expectedImageInfo = realityData.GetImageInfo();
    ImageUtilities::RgbImageInfo actualImageInfo = realityData.GetImageInfo();
    Utf8String contentType = realityData.GetContentType();
    
    BentleyStatus status;

    m_rgbBuffer.clear(); // reuse the same buffer, in order to minimize mallocs

    if (contentType.Equals ("image/png"))
        {
        status = ImageUtilities::ReadImageFromPngBuffer (m_rgbBuffer, actualImageInfo, data.data(), data.size());
        if (SUCCESS != status)
            LOG.warningv("Invalid png image data: %s", url.c_str());
        }
    else if (contentType.Equals ("image/jpeg"))
        {
        status = ImageUtilities::ReadImageFromJpgBuffer (m_rgbBuffer, actualImageInfo, data.data(), data.size(), expectedImageInfo);
        if (SUCCESS != status)
            LOG.warningv("Invalid jpeg image data: %s", url.c_str());
        }
    else
        {
        BeAssert (false && "Unsupported image type");
        LOG.warningv("Unsupported image type: %s -> %s", url.c_str(), contentType.c_str());
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
    DrawTileAsBox (context, tileid, -DgnViewport::GetDisplayPriorityFrontPlane(), false);
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
    TextStringPropertiesPtr props = TextStringProperties::Create (DgnFontManager::GetLastResortTrueTypeFont(), NULL, textScale);
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
BentleyStatus WebMercatorTilingSystem::GetOptimalZoomLevelForView (uint8_t& zoomLevel, DgnViewportR vp, WebMercatorUorConverter& converter, bool preferFinerResolution)
    {
    // Get the number of meters / screen pixel that we are showing in the view
    double viewResolution = converter.ComputeViewResolutionInMetersPerPixel (vp); 

    // Return the zoom level that has about the same "ground resolution", which is defined as meters / pixel.
    for (uint8_t i=0; i<=MAX_ZOOM_LEVEL; ++i)
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
BentleyStatus WebMercatorTilingSystem::GetTileIdsForView (bvector<TileId>& tileids, uint8_t zoomLevel, DgnViewportR vp, WebMercatorUorConverter& converter)
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
            LOG.info("The view is not looking at any part of the x-y plane");
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
* @bsimethod                                                    Sam.Wilson      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorTilingSystem::TileId const& tileid)
    {
    ModelHandlerP modelHandler = dgn_ModelHandler::Model::FindHandler(m_model.GetDgnDb(), m_model.GetClassId());
    dgn_ModelHandler::WebMercator* webMercatorModelHandler = dynamic_cast<dgn_ModelHandler::WebMercator*>(modelHandler);
    if (nullptr == webMercatorModelHandler)
        {
        BeAssert(false);
        return BSIERROR;
        }
    return webMercatorModelHandler->_CreateUrl(url, imageInfo, m_model.m_mercator, tileid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WebMercatorDisplay::DrawSubstituteTilesFinerFromTextureCache (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, uint32_t maxLevelsToTry)
    {
    if (tileid.zoomLevel >= WebMercatorTilingSystem::MAX_ZOOM_LEVEL)
        return BSIERROR;

    // Get tiles at the next finer resolution to fill in the space defined by the specified tileid

    WebMercatorTilingSystem::TileId finerUlTileid (tileid); // The tile in the upper left corner of the tile that we need to fill.
    ++finerUlTileid.zoomLevel;
    finerUlTileid.row <<= 1;
    finerUlTileid.column <<= 1;

    for (uint32_t row=0; row < 2; ++row)
        {
        for (uint32_t col=0; col < 2; ++col)
            {
            WebMercatorTilingSystem::TileId finerTileid (finerUlTileid);
            finerTileid.column += col;
            finerTileid.row += row;

            Utf8String finerUrl;
            ImageUtilities::RgbImageInfo finerImageInfo;
            if (CreateUrl (finerUrl, finerImageInfo, finerTileid) != BSISUCCESS)
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
BentleyStatus WebMercatorDisplay::DrawCoarserTilesForViewFromTextureCache (ViewContextR context, uint8_t zoomLevel, uint32_t maxLevelsToTry)
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
BentleyStatus WebMercatorDisplay::GetCachedTiles (bvector<TileDisplayImageData>& tileDisplayImageData, bool& allFoundInTextureCache, uint8_t zoomLevel, ViewContextR context)
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
        if (CreateUrl (url, data.imageInfo, data.tileid) != BSISUCCESS)
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
void WebMercatorModel::_AddGraphicsToScene (ViewContextR context)
    {
    RefCountedPtr<WebMercatorDisplay> display = new WebMercatorDisplay(*this, *context.GetViewport());
    display->DrawView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorDisplay::DrawView (ViewContextR context)
    {
    // **********************************
    // *** NB: This method must be fast. 
    // **********************************
    // Do not try to read from SQLite or allocate huge amounts of memory in here. 
    // Defer time-consuming tasks to progressive display

    //
    //  First, determine if we can draw map tiles at all.
    //
    if (!shouldDraw (context) || NULL == context.GetViewport())
        return;

    if (context.GetViewport()->IsCameraOn())    // *** TBD: Not sure if we can support tiled raster in a perspective view or not. 
        return;                                 // ***      I would at least have to figure out how to compute what portions of the earth are in the view.

    m_hadError = true;

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
        #ifdef WEBMERCATOR_DEBUG_TILES
            m_helper.DrawTileDebugInfo (context, tileddata.tileid);
        #endif

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
IProgressiveDisplay::Completion WebMercatorDisplay::_Process (ViewContextR context)
    {
    if (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < m_nextRetryTime)
        {
        LOG.tracev("Wait %lld until next retry", m_nextRetryTime - BeTimeUtilities::GetCurrentTimeAsUnixMillis());
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
        CreateUrl (url, expectedImageInfo, tileid);
        auto realityData = T_HOST.GetRealityDataAdmin().GetCache().Get<TiledRaster>(url.c_str(), *TiledRaster::RequestOptions::Create(expectedImageInfo, true));
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
        auto realityData = T_HOST.GetRealityDataAdmin().GetCache().Get<TiledRaster>(url.c_str(), *TiledRaster::RequestOptions::Create(true));

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
        m_waitTime = (uint64_t)(m_waitTime * 1.33);
        if (m_failedAttempts > 5 && (m_failedAttempts&1)==0)
            {
            LOG.infov("%d failures. Wait %lld millis\n", m_failedAttempts, m_waitTime);
            }
        m_nextRetryTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() + m_waitTime;  
        }

    //  Don't report "Finished" unless all missing tiles have been found and displayed.
    return m_missingTilesPending.empty()? Completion::Finished: Completion::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorDisplay::WebMercatorDisplay (WebMercatorModel& model, DgnViewportR vp) 
    :
    m_model(model),
    m_optimalZoomLevel(0), 
    m_hadError(false),
    m_preferFinerResolution(false),
    m_drawSubstitutes(s_drawSubstitutes),
    m_helper(vp),
    m_waitTime(0),
    m_nextRetryTime(0),
    m_failedAttempts(0)
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorDisplay::~WebMercatorDisplay() 
    {
    }

#ifdef WIP_MAP_SERVICE
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
Utf8String StreetMapModelHandler::CreateBingUrl (WebMercatorTilingSystem::TileId const& tileid)
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
Utf8String StreetMapModelHandler::CreateGoogleMapsUrl (WebMercatorTilingSystem::TileId const& tileid)
    {
    // *** WIP_WEBMERCATOR We must append an API key -- need to get one.
    // *** WIP_WEBMERCATOR m_mapType
    // *** WIP_WEBMERCATOR m_features
    return Utf8PrintfString ("http://mts0.googleapis.com/vt?lyrs=m&x=%d&y=%d&z=%d", tileid.column, tileid.row, tileid.zoomLevel);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String dgn_ModelHandler::StreetMap::CreateMapquestUrl (WebMercatorTilingSystem::TileId const& tileid, WebMercatorModel::Mercator const& props)
    {
    Utf8String url = "http://otile1.mqcdn.com/tiles/1.0.0/";
    if (!props.m_mapType.empty() && props.m_mapType[0] == '0')
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
BentleyStatus dgn_ModelHandler::StreetMap::_CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& expectedImageInfo, WebMercatorModel::Mercator const& props, WebMercatorTilingSystem::TileId const& tileid)
    {
    // The usual image format info
    expectedImageInfo.height = expectedImageInfo.width = 256;
    expectedImageInfo.hasAlpha = false;
    expectedImageInfo.isBGR = false;
    expectedImageInfo.isTopDown = true;

    if (props.m_mapService.empty())
        {
        BeAssert (false && "missing map service");
        LOG.error("missing map service");
        return BSIERROR;
        }

#ifdef WIP_MAPQUEST_NO_LONGER_AVAILABLE
    if (props.m_mapService[0] == '0') // "(c) OpenStreetMap contributors"
        url = CreateMapquestUrl (tileid, props);
#else
    #ifndef NDEBUG  // *** In a developer build, go ahead and use OSM
    if (true)
        url = Utf8PrintfString ("http://tile.openstreetmap.org/%d/%d/%d.png", tileid.zoomLevel, tileid.column, tileid.row);
    #endif
#endif
#ifdef WIP_MAP_SERVICE
    else
    if (OpenStreetMaps)
        url = Utf8PrintfString ("http://tile.openstreetmap.org/%d/%d/%d.png", tileid.zoomLevel, tileid.column, tileid.row);
    else if (GoogleMaps)
        url = CreateGoogleMapsUrl (tileid);
    else if (BlackAndWhite)
        url = Utf8PrintfString ("http://a.tile.stamen.com/toner/%d/%d/%d.png", tileid.zoomLevel, tileid.column, tileid.row);
    else if (Bing)
        url = CreateBingUrl (tileid);
#endif
    else
        {
        BeAssert (false && "unrecognized map service");
        LOG.errorv("[%s] is an unrecognized map service", props.m_mapService.c_str());
        return BSIERROR;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getStreetMapServerDescription(dgn_ModelHandler::StreetMap::MapService mapService, dgn_ModelHandler::StreetMap::MapType mapType)
    {
    Utf8String descr;
    switch (mapService)
        {
        case dgn_ModelHandler::StreetMap::MapService::MapQuest:
            {
            descr = ("MapQuest");   // *** WIP translate
            if (dgn_ModelHandler::StreetMap::MapType::Map == mapType)
                descr.append(" Map");   // *** WIP translate
            else
                descr.append(" Satellite Images"); // *** WIP translate
            break;
            }
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
    Utf8String modelName = getStreetMapServerDescription(mapService,mapType).c_str();
    DgnClassId classId(db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "StreetMapModel"));
    BeAssert(classId.IsValid());

    WebMercatorModelPtr model = new StreetMapModel(DgnModel::CreateParams(db, classId, modelName.c_str()));

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
void WebMercatorModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_mercator.ToJson(v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_mercator.FromJson(v);
    }

#ifdef TBD_LATLNG_GRID
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void LatLongGridRealityDataHandler::_AddGraphicsToScene (ViewContextR context)
    {
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
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP TiledRaster::_GetId() const {return m_url.c_str();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool TiledRaster::_IsExpired() const {return DateTime::CompareResult::EarlierThan == DateTime::Compare(GetExpirationDate(), DateTime::GetCurrentTime());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Byte> const& TiledRaster::GetData() const {return m_data;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime TiledRaster::GetCreationDate() const {return m_creationDate;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageUtilities::RgbImageInfo const& TiledRaster::GetImageInfo() const {return m_rasterInfo;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TiledRaster::GetContentType() const {return m_contentType;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<TiledRaster> TiledRaster::Create() {return new TiledRaster();}

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            03/2015
//=======================================================================================
struct TiledRasterPrepareAndCleanupHandler : BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandler
    {
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                     Grigas.Petraitis           03/2015
    +---------------+---------------+---------------+---------------+---------------+--*/
    virtual BentleyStatus _PrepareDatabase(BeSQLite::Db& db) const override
        {
        if (db.TableExists(TABLE_NAME_TiledRaster))
            return SUCCESS;
    
        Utf8CP ddl = "Url CHAR PRIMARY KEY, \
                        Raster BLOB,          \
                        RasterSize INT,       \
                        RasterInfo CHAR,      \
                        ContentType CHAR,     \
                        Created BIGINT,       \
                        Expires BIGINT,       \
                        ETag CHAR";
        return BeSQLite::BE_SQLITE_OK == db.CreateTable(TABLE_NAME_TiledRaster, ddl) ? SUCCESS : ERROR;
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
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "select RasterSize, Created from " TABLE_NAME_TiledRaster " ORDER BY Created ASC"))
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr TiledRaster::_GetDatabasePrepareAndCleanupHandler() const
    {
    return new TiledRasterPrepareAndCleanupHandler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& requestOptions) 
    {
    BeAssert(nullptr != dynamic_cast<RequestOptions const*>(&requestOptions));
    RequestOptions const& options = static_cast<RequestOptions const&>(requestOptions);
    
    m_url.AssignOrClear(url);
    m_creationDate = DateTime::GetCurrentTime();

    auto contentTypeIter = header.find("Content-Type");
    if (contentTypeIter == header.end())
        return ERROR;

    m_contentType = contentTypeIter->second.c_str();
    BeAssert(nullptr != options.GetExpectedImageInfo());
    m_rasterInfo = *options.GetExpectedImageInfo();
    m_data = body;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledRaster::_InitFrom(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options)
    {
    HighPriorityOperationBlock highPriority;

    CachedStatementPtr stmt;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "SELECT Raster, RasterSize, RasterInfo, Created, Expires, ETag, ContentType from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    stmt->ClearBindings();
    stmt->BindText(1, key, BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        m_url = key;

        auto raster     = stmt->GetValueBlob(0);
        auto rasterSize = stmt->GetValueInt(1);
        m_data.assign((Byte*) raster,(Byte*) raster + rasterSize);

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
BentleyStatus TiledRaster::_Persist(BeSQLite::Db& db) const
    {
    int bufferSize = (int) GetData().size();

    int64_t creationTime = 0;
    if (SUCCESS != GetCreationDate().ToUnixMilliseconds(creationTime))
        return ERROR;

    int64_t expirationDate = 0;
    if (SUCCESS != GetExpirationDate().ToUnixMilliseconds(expirationDate))
        return ERROR;

    HighPriorityOperationBlock highPriority;

    CachedStatementPtr selectStatement;
    if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(selectStatement, "SELECT Url from " TABLE_NAME_TiledRaster " WHERE Url=?"))
        return ERROR;

    selectStatement->ClearBindings();
    selectStatement->BindText(1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
    if (BeSQLite::BE_SQLITE_ROW == selectStatement->Step())
        {
        // update
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "UPDATE " TABLE_NAME_TiledRaster " SET Expires=?, ETag=? WHERE Url=?"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindInt64(1, expirationDate);
        stmt->BindText (2, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText (3, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }
    else
        {
        // insert
        CachedStatementPtr stmt;
        if (BeSQLite::BE_SQLITE_OK != db.GetCachedStatement(stmt, "INSERT INTO " TABLE_NAME_TiledRaster "(Url, Raster, RasterSize, RasterInfo, Created, Expires, ETag, ContentType) VALUES(?,?,?,?,?,?,?,?)"))
            return ERROR;

        stmt->ClearBindings();
        stmt->BindText (1, GetId(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindBlob (2, GetData().data(), bufferSize, BeSQLite::Statement::MakeCopy::No);
        stmt->BindInt  (3, bufferSize);
        stmt->BindText (4, SerializeRasterInfo(m_rasterInfo).c_str(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindInt64(5, creationTime);
        stmt->BindInt64(6, expirationDate);
        stmt->BindText (7, GetEntityTag(), BeSQLite::Statement::MakeCopy::Yes);
        stmt->BindText (8, GetContentType(), BeSQLite::Statement::MakeCopy::Yes);
        if (BeSQLite::BE_SQLITE_DONE != stmt->Step())
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TiledRaster::SerializeRasterInfo(ImageUtilities::RgbImageInfo const& info)
    {
    Json::Value json;
    json["hasAlpha"] = info.hasAlpha;
    json["height"] = info.height;
    json["width"] = info.width;
    json["isBGR"] = info.isBGR;
    json["isTopDown"] = info.isTopDown;
    return Json::FastWriter::ToString(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImageUtilities::RgbImageInfo TiledRaster::DeserializeRasterInfo(Utf8CP serializedJson)
    {
    Json::Value json;
    Json::Reader reader;
    reader.parse(serializedJson, json);

    ImageUtilities::RgbImageInfo info;
    info.hasAlpha = json["hasAlpha"].asBool();
    info.height = json["height"].asInt();
    info.width = json["width"].asInt();
    info.isBGR = json["isBGR"].asBool();
    info.isTopDown = json["isTopDown"].asBool();
    return info;
    }