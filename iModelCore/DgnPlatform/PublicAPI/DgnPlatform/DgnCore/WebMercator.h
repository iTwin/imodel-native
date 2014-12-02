/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/WebMercator.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnCore/DgnViewport.h>
#include <DgnPlatform/DgnCore/DgnProjectTables.h>

//#define WEBMERCATOR_DEBUG_TILES

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct WebMercatorUorConverter;

//=======================================================================================
// Utility for computing WebMercator tile ids
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorTilingSystem
{
    //! Identifies a tile in the fixed WebMercator tiling system.
    //! Note that the tile 0,0 is always in the upper left corner of the WebMercator projection.
    struct TileId
        {
        UInt8       zoomLevel;
        UInt32      column;
        UInt32      row;
        };

    enum {MIN_ZOOM_LEVEL=0, MAX_ZOOM_LEVEL=22};
    
    static UInt8 GetZoomLevels() {return (UInt8)(MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL);}
    //! Figure out what zoom level to use. The basic algorithm is to use tiles with about the same meters/pixel resolution as the
    //! Dgn view is using for model data. Normally, we find two zoom levels, one with less and the other with more resolution.
    //! The preferFinerResolution parameter specifies which to use. The finer the resolution, the more tiles must be downloaded and displayed.
    static BentleyStatus GetOptimalZoomLevelForView (UInt8& zoomLevel, ViewportR, WebMercatorUorConverter&, bool preferFinerResolution);
    static BentleyStatus GetTileIdsForView (bvector<TileId>& tileids, UInt8 desiredZoomLevel, ViewportR, WebMercatorUorConverter&);
};

#define WEBMERCATOR_COMPARE_VALUES(val0, val1)  if (val0 < val1) { return true; } if (val0 > val1) { return false; }

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

//=======================================================================================
// Utility for converting between DgnProject coordinates and WebMercator coordinates.
// Conversions are based on the ground resolution (i.e., meters/pixel) at the latitude 
// of the DgnProject's geo origin. That same ground resolution is used for coordinates
// in all parts of the WebMercator projection.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorUorConverter
    {
    DgnUnits& m_units;
    double m_originLatitudeInRadians;
    double m_meters_per_uor;
    bmap<DPoint3d, GeoPoint, CompareDPoint3dLess> m_uorsToLatLng;
    bmap<GeoPoint, DPoint3d, CompareGeoPointLess> m_latLngToUors;

    WebMercatorUorConverter (ViewportR);

    bool IsValid() const {return m_units.GetDgnGCS() != NULL;}

    BentleyStatus ComputeTileCorners (DPoint3d* corners, WebMercatorTilingSystem::TileId const& tileid);

    //! Compute how many meters per pixel are displayed in the view. We measure the diagonals.
    double ComputeViewResolutionInMetersPerPixel (ViewportR);

    //! Compute meters/pixel at the DgnProject's geo origin the specified zoomLevel.
    double ComputeGroundResolutionInMeters (UInt8 zoomLevel);

    //! Convert a DPoint3d expressed in the Project's UOR coordinate system to a GeoPoint.
    GeoPoint ConvertUorsToLatLng (DPoint3dCR pt);

    //! Convert a GeoPoint to a point expressed in the Project's UOR coordinate system.
    DPoint3d ConvertLatLngToUors (GeoPoint gp);

    //! Convert a DPoint2d expressed in the Project's UOR coordinate system to a DPoint2d expressed in wpixels.
    DPoint2d ConvertUorsToWpixels (DPoint2dCR pt);
    };

//=======================================================================================
// Utility for displaying WebMercator tiles
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorTileDisplayHelper
    {
//protected:
    WebMercatorUorConverter m_converter;
    bvector<byte> m_rgbBuffer;
    bool m_drawingSubstituteTiles;
public:

    //! Construct a helper object
    //! @param[in] vp      The viewport
    WebMercatorTileDisplayHelper (ViewportR vp);

    //! Define a texture to represent an image in preparation for calling DrawTile. When it defines a texture,
    //! QuickVision converts the raw image data into a form that is ready to be displayed by the native graphics card.
    //! @note You must either call CacheTexture to take ownership of the returned texture, or you must delete the texture yourself when you are done with it.
    //! @note This function will fail with unpredictable results if the image is bigger than about 2048x2048.
    //! @return a unique ID to identify the texture
    //! @param[in] rgbData  The raw image data
    //! @param[in] imageInfo Defines the format, size, and orientation of the raw image data
    static uintptr_t DefineTexture (bvector<byte> const& rgbData, ImageUtilities::RgbImageInfo const& imageInfo);

    //! Add texture to temporary cache, or if a texture is already cached for this url, then replace it.
    //! @note Do not delete the texture. The cache will delete it if and when it is removed by trim or by replacement.
    //! @param[in] url  The key
    //! @param[in] textureId Identifies the texture
    //! @param[in] imageInfo Additional info about the image stored in the texture
    static void CacheTexture (Utf8StringCR url, uintptr_t textureId, ImageUtilities::RgbImageInfo const& imageInfo);

    //! Get an existing texture from the cache.
    static BentleyStatus GetCachedTexture (uintptr_t& cachedTextureId, ImageUtilities::RgbImageInfo& cachedImageInfo, Utf8StringCR url);

    //! Draw the specified tile
    //! @param[in] context      The viewcontext
    //! @param[in] tileid       The tile id
    //! @param[in] imageInfo    Information about the image
    //! @param[in] textureId    The texture id to use
    void DrawTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, ImageUtilities::RgbImageInfo const& imageInfo, uintptr_t textureId);

    void DrawMissingTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid);

    void DrawAndCacheTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, Utf8StringCR url, IRealityData& realityData);

    #ifdef WEBMERCATOR_DEBUG_TILES
    void DrawTileDebugInfo (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid);
    void DrawTileAsBox (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, double z, bool filled);
    #endif
    };

//=======================================================================================
// Obtains and displays multi-resolution tiled raster reality data that is organized
// according to the WebMercator tiling system
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorRealityDataHandler : IRealityDataHandler, IProgressiveDisplay
{
protected:
    bvector<WebMercatorTilingSystem::TileId> m_missingTilesToBeRequested;    //!< Tiles that are missing and that should be requested
    bmap<Utf8String, WebMercatorTilingSystem::TileId> m_missingTilesPending; //!< Tiles that are missing and that we are waiting for.
    UInt32 m_failedAttempts;
    UInt64 m_nextRetryTime;                             //!< When to re-try m_missingTilesPending. unix millis UTC
    UInt64 m_waitTime;                                  //!< How long to wait before re-trying m_missingTilesPending. millis 
    UInt8 m_optimalZoomLevel;                           //!< The zoomLevel that would be optimal for the view. Not necessarily used by every tile.
    bool m_hadError;                                    //!< If true, no tiles could be found or displayed.
    bool m_drawSubstitutes;
    bool m_preferFinerResolution;                       //!< Download and display more tiles, in order to get the best resolution? Else, go with 1/4 as many tiles and be satisfied with slightly fuzzy resolution.
    WebMercatorTileDisplayHelper m_helper;

    BentleyStatus DrawSubstituteTilesFinerFromTextureCache (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, UInt32 maxLevelsToTry);
    BentleyStatus DrawCoarserTilesForViewFromTextureCache (ViewContextR context, UInt8 zoomLevel, UInt32 maxLevelsToTry);

    struct TileDisplayImageData
        {
        WebMercatorTilingSystem::TileId tileid;
        ImageUtilities::RgbImageInfo imageInfo;
        uintptr_t textureId;
        };

    BentleyStatus GetCachedTiles (bvector<TileDisplayImageData>& tilesAndUrls, bool& allFoundInTextureCache, UInt8 zoomLevel, ViewContextR context);

    //! Displays tiled rasters and schedules progressive display.
    //! This function displays only the tiled images that are immediately available. If any tile is not
    //! available from the cache, this function requests that the tile should be downloaded, 
    //! and then schedules this object for progressive display. 
    //! OUTPUT: This function populates m_missingTiles.
    DGNPLATFORM_EXPORT virtual void _DrawView (ViewContextR) override;

    //! Displays tiled rasters and schedules downloads. 
    //! INPUT: This function assumes that m_missingTiles has been populated.
    //! This function removes items from m_missingTiles that can be displayed.
    //! This function does not request tiles to be downloaded.
    //! This function returns Finished if m_missingTiles becomes empty.
    //! This function stops whenever view.CheckStop is true.
    //! OUTPUT: This function removes 0 or more items from m_missingTiles.
    DGNPLATFORM_EXPORT virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    DGNPLATFORM_EXPORT virtual bool _WantTimeoutSet(UInt32& limit) override {return false;}

    //! A subclass must override this method to create the URL used by a particular tile server.
    //! _Process calls this function and passes the result to the tile cache when it requests a tiled image.
    //! @param[out] url         The tile URL that is ready to be used in an HTTP GET operation. This URL is also used to uniquely identify the tile in the tile cache.
    //! @param[out] imageInfo   Information about the image format that is expected to be the result of the HTTP GET request. The downloaded data will include the image type. What we need to know is: is the image top-down, does it have an alpha channel? Is it in BGR or RGB byte order? And things like that.
    //! @param[in]  tileid      Identifies the tile
    virtual BentleyStatus _CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorTilingSystem::TileId const& tileid) = 0;

    //! Construct an instance of a WebMercatorRealityDataHandler.
    //! @param[in] vp           The viewport that is to display the map
    DGNPLATFORM_EXPORT WebMercatorRealityDataHandler (ViewportR vp);
};

//=======================================================================================
// Obtains and displays street map reality data, which is a type of WebMercator
// multi-resolution tiled raster data.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct StreetMapRealityDataHandler : WebMercatorRealityDataHandler
{
    //! Identifies a well known street map tile service
    enum class MapService 
        {
        GoogleMaps,             //!< Google maps
        MapQuest,               //!< MapQuest
        BlackAndWhite,          //!< Black and white maps
        Bing,                   //!< Bing
        #ifndef NDEBUG
        TEST_OpenStreetMaps,    //!< Open Street Maps
        #endif
        };

    //! The kind of map to display
    enum class MapType
        {
        Map,                    //!< Show a map
        SatelliteImage,         //!< Show a satellite image (not supported by all MapServices)
        };

    //! The kinds of information that should be displayed on top of the base map
    enum class Feature
        {
        Roads,                  //!< Roads
        /* TBD...*/
        };

    MapType                     m_mapType;  //!< Select the kind of map you want
    bmap<Feature,Utf8String>    m_features; //!< Select the stuff that should appear on the map

protected:
    MapService m_mapService;

    DGNPLATFORM_EXPORT Utf8String CreateBingUrl (WebMercatorTilingSystem::TileId const&);
    DGNPLATFORM_EXPORT Utf8String CreateGoogleMapsUrl (WebMercatorTilingSystem::TileId const&);
    DGNPLATFORM_EXPORT Utf8String CreateMapquestUrl (WebMercatorTilingSystem::TileId const&);

    DGNPLATFORM_EXPORT virtual BentleyStatus _CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorTilingSystem::TileId const&);

    DGNPLATFORM_EXPORT StreetMapRealityDataHandler (ViewportR, MapService);
    
public:
    //! Create an instance of a StreeMapRealityDataHandler
    //! @param[in] vp           The viewport that is to display the map
    //! @param[in] mapService   The street map service to use
    static RefCountedPtr<StreetMapRealityDataHandler> Create (ViewportR vp, MapService mapService) {return new StreetMapRealityDataHandler(vp,mapService);}

    //! Set the imagery you want
    DGNPLATFORM_EXPORT void SetMapType (MapType);

    //! Add a feature
    DGNPLATFORM_EXPORT void SetFeature (Feature, Utf8StringCR);

    //! Specify that you want the reality data handler to download and display more and smaller tiles, if necessary, in order to get the best resolution.
    //! The default is to display fewer tiles and sometimes display slightly fuzzy resolution.
    DGNPLATFORM_EXPORT void SetPreferFinerResolution (bool);

    //! Remove a feature
    DGNPLATFORM_EXPORT void RemoveFeature (Feature);
};

//=======================================================================================
// Displays a grid of latitude and longitude lines
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct LatLongGridRealityDataHandler : IRealityDataHandler
{
protected:
    DGNPLATFORM_EXPORT virtual void _DrawView (ViewContextR) override;

};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
