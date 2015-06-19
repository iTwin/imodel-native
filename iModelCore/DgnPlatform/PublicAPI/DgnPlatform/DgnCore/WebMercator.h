/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/WebMercator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnCore/DgnViewport.h>
#include <DgnPlatform/DgnCore/DgnDbTables.h>

//#define WEBMERCATOR_DEBUG_TILES

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct WebMercatorUorConverter;
struct WebMercatorModel;
struct WebMercatorModelHandler;

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
        uint8_t     zoomLevel;
        uint32_t    column;
        uint32_t    row;
        };

    enum {MIN_ZOOM_LEVEL=0, MAX_ZOOM_LEVEL=22};
    
    static uint8_t GetZoomLevels() {return (uint8_t)(MAX_ZOOM_LEVEL - MIN_ZOOM_LEVEL);}
    //! Figure out what zoom level to use. The basic algorithm is to use tiles with about the same meters/pixel resolution as the
    //! Dgn view is using for model data. Normally, we find two zoom levels, one with less and the other with more resolution.
    //! The preferFinerResolution parameter specifies which to use. The finer the resolution, the more tiles must be downloaded and displayed.
    static BentleyStatus GetOptimalZoomLevelForView (uint8_t& zoomLevel, DgnViewportR, WebMercatorUorConverter&, bool preferFinerResolution);
    static BentleyStatus GetTileIdsForView (bvector<TileId>& tileids, uint8_t desiredZoomLevel, DgnViewportR, WebMercatorUorConverter&);
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
// Utility for converting between DgnDb coordinates and WebMercator coordinates.
// Conversions are based on the ground resolution (i.e., meters/pixel) at the latitude 
// of the DgnDb's geo origin. That same ground resolution is used for coordinates
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

    WebMercatorUorConverter (DgnViewportR);

    bool IsValid() const {return m_units.GetDgnGCS() != NULL;}

    BentleyStatus ComputeTileCorners (DPoint3d* corners, WebMercatorTilingSystem::TileId const& tileid);

    //! Compute how many meters per pixel are displayed in the view. We measure the diagonals.
    double ComputeViewResolutionInMetersPerPixel (DgnViewportR);

    //! Compute meters/pixel at the DgnDb's geo origin the specified zoomLevel.
    double ComputeGroundResolutionInMeters (uint8_t zoomLevel);

    //! Convert a DPoint3d expressed in the Project's UOR coordinate system to a GeoPoint.
    GeoPoint ConvertUorsToLatLng (DPoint3dCR pt);

    //! Convert a GeoPoint to a point expressed in the Project's UOR coordinate system.
    DPoint3d ConvertLatLngToUors (GeoPoint gp);

    //! Convert a DPoint2d expressed in the Project's UOR coordinate system to a DPoint2d expressed in wpixels.
    DPoint2d ConvertUorsToWpixels (DPoint2dCR pt);
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct TiledRaster : IRealityData<TiledRaster, BeSQLiteRealityDataStorage, HttpRealityDataSource>
    {
    //===================================================================================
    // @bsiclass                                        Grigas.Petraitis        10/2014
    //===================================================================================
    struct RequestOptions : RealityDataCacheOptions, IRealityData::RequestOptions
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS
    private:
        ImageUtilities::RgbImageInfo const* m_expectedImageInfo;
        RequestOptions(bool allowExpired) : m_expectedImageInfo(nullptr), RealityDataCacheOptions(allowExpired, false) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
        RequestOptions(ImageUtilities::RgbImageInfo const& expectedImageInfo, bool allowExpired) : m_expectedImageInfo(new ImageUtilities::RgbImageInfo(expectedImageInfo)), RealityDataCacheOptions(allowExpired, true) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    public:
        ~RequestOptions() {DELETE_AND_CLEAR(m_expectedImageInfo);}
        static RefCountedPtr<RequestOptions> Create(bool allowExpired = true) {return new RequestOptions(allowExpired);}
        static RefCountedPtr<RequestOptions> Create(ImageUtilities::RgbImageInfo const& expectedImageInfo, bool allowExpired = true) {return new RequestOptions(expectedImageInfo, allowExpired);}
        ImageUtilities::RgbImageInfo const* GetExpectedImageInfo() const {return m_expectedImageInfo;}
    };

private:
    Utf8String      m_url;
    bvector<Byte>   m_data;
    DateTime        m_creationDate;
    Utf8String      m_contentType;
    ImageUtilities::RgbImageInfo m_rasterInfo;

private:
    static Utf8String SerializeRasterInfo(ImageUtilities::RgbImageInfo const&);
    static ImageUtilities::RgbImageInfo DeserializeRasterInfo(Utf8CP);

    TiledRaster(){}
        
protected:
    virtual Utf8CP _GetId() const override;
    virtual bool _IsExpired() const override;
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override;
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options) override;
    virtual BentleyStatus _Persist(BeSQLite::Db& db) const override;
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override;

public:
    static RefCountedPtr<TiledRaster> Create();    
    bvector<Byte> const& GetData() const;
    DateTime GetCreationDate() const;
    ImageUtilities::RgbImageInfo const& GetImageInfo() const;
    Utf8String GetContentType() const;
    };

//=======================================================================================
// Utility for displaying WebMercator tiles
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorTileDisplayHelper
    {
//protected:
    WebMercatorUorConverter m_converter;
    bvector<Byte> m_rgbBuffer;
    bool m_drawingSubstituteTiles;
public:

    //! Construct a helper object
    //! @param[in] vp      The viewport
    WebMercatorTileDisplayHelper (DgnViewportR vp);

    //! Define a texture to represent an image in preparation for calling DrawTile. When it defines a texture,
    //! QuickVision converts the raw image data into a form that is ready to be displayed by the native graphics card.
    //! @note You must either call CacheTexture to take ownership of the returned texture, or you must delete the texture yourself when you are done with it.
    //! @note This function will fail with unpredictable results if the image is bigger than about 2048x2048.
    //! @return a unique ID to identify the texture
    //! @param[in] rgbData  The raw image data
    //! @param[in] imageInfo Defines the format, size, and orientation of the raw image data
    static uintptr_t DefineTexture (bvector<Byte> const& rgbData, ImageUtilities::RgbImageInfo const& imageInfo);

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

    void DrawAndCacheTile (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, Utf8StringCR url, TiledRaster& realityData);

    #ifdef WEBMERCATOR_DEBUG_TILES
    void DrawTileDebugInfo (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid);
    void DrawTileAsBox (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, double z, bool filled);
    #endif
    };

//=======================================================================================
// Dislays tiles of a street map as they become available over time.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct WebMercatorDisplay : IProgressiveDisplay, NonCopyableClass
{
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

    friend struct WebMercatorModel;

protected:
    WebMercatorModel& m_model;              //!< The model
    bvector<WebMercatorTilingSystem::TileId> m_missingTilesToBeRequested;    //!< Tiles that are missing and that should be requested
    bmap<Utf8String, WebMercatorTilingSystem::TileId> m_missingTilesPending; //!< Tiles that are missing and that we are waiting for.
    uint32_t m_failedAttempts;
    uint64_t m_nextRetryTime;                             //!< When to re-try m_missingTilesPending. unix millis UTC
    uint64_t m_waitTime;                                  //!< How long to wait before re-trying m_missingTilesPending. millis 
    uint8_t m_optimalZoomLevel;                           //!< The zoomLevel that would be optimal for the view. Not necessarily used by every tile.
    bool m_hadError;                                    //!< If true, no tiles could be found or displayed.
    bool m_drawSubstitutes;
    bool m_preferFinerResolution;                       //!< Download and display more tiles, in order to get the best resolution? Else, go with 1/4 as many tiles and be satisfied with slightly fuzzy resolution.
    WebMercatorTileDisplayHelper m_helper;

protected:
    BentleyStatus DrawSubstituteTilesFinerFromTextureCache (ViewContextR context, WebMercatorTilingSystem::TileId const& tileid, uint32_t maxLevelsToTry);
    BentleyStatus DrawCoarserTilesForViewFromTextureCache (ViewContextR context, uint8_t zoomLevel, uint32_t maxLevelsToTry);

    struct TileDisplayImageData
        {
        WebMercatorTilingSystem::TileId tileid;
        ImageUtilities::RgbImageInfo imageInfo;
        uintptr_t textureId;
        };

    BentleyStatus GetCachedTiles (bvector<TileDisplayImageData>& tilesAndUrls, bool& allFoundInTextureCache, uint8_t zoomLevel, ViewContextR context);

    //! Helper function that invokes _CreateUrl on the handler
    DGNPLATFORM_EXPORT virtual BentleyStatus CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorTilingSystem::TileId const& tileid);

    //! Displays tiled rasters and schedules downloads. 
    //! INPUT: This function assumes that m_missingTiles has been populated.
    //! This function removes items from m_missingTiles that can be displayed.
    //! This function does not request tiles to be downloaded.
    //! This function returns Finished if m_missingTiles becomes empty.
    //! This function stops whenever view.CheckStop is true.
    //! OUTPUT: This function removes 0 or more items from m_missingTiles.
    DGNPLATFORM_EXPORT virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    DGNPLATFORM_EXPORT virtual bool _WantTimeoutSet(uint32_t& limit) override {return false;}

    DGNPLATFORM_EXPORT void DrawView (ViewContextR);

    //! Construct an instance of a WebMercatorRealityDataHandler.
    //! @param[in] model        The WebMercatorModel
    //! @param[in] vp           The viewport that is to display the map
    DGNPLATFORM_EXPORT WebMercatorDisplay (WebMercatorModel& model, DgnViewportR vp);

    DGNPLATFORM_EXPORT ~WebMercatorDisplay();
};

//=======================================================================================
// Obtains and displays multi-resolution tiled raster reality data that is organized
// according to the WebMercator tiling system. Collaborates with WebMercatorModelHandler
// in order to send requests to a tile server.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : PhysicalModel
{
    DEFINE_T_SUPER(PhysicalModel)

public:
    struct Mercator
        {
        AxisAlignedBox3d m_range;             //! The range covered by this map -- typically the project's extents -- could be the whole world
        Utf8String m_mapService;        //! Identifies the source of the tiled map data. This is a token that is supplied by the 
                                        //! subclass of WebMercatorModelHandler and stored on a WebMercatorModel instande in the DgnDb, 
                                        //! in order to associate it with a map server. The WebMercatorModelHandler subclass looks at this string
                                        //! when constructing URLs at runtime for requesting tiles for the model.
        Utf8String m_mapType;           //! Identifies the type of map data to request and display.
        bool       m_finerResolution;   //! true => download and display more and smaller tiles, if necessary, in order to get the best resolution.
                                        //! false => download and display fewer and larger tiles, resulting in sometimes slightly fuzzy resolution.

        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

protected:
    friend struct WebMercatorModelHandler;
    friend struct WebMercatorDisplay;

    Mercator m_mercator;

public:
    //! Create a new WebMercatorModel object, in preparation for loading it from the DgnDb.
    WebMercatorModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT virtual void _AddGraphicsToScene(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _ToPropertiesJson(Json::Value&) const override;
    DGNPLATFORM_EXPORT virtual void _FromPropertiesJson(Json::Value const&) override;
    virtual AxisAlignedBox3d _QueryModelRange() const override {return m_mercator.m_range;}

    //! Call this after creating a new model, in order to set up subclass-specific properties.
    void SetMercator(Mercator const&);
};

namespace dgn_ModelHandler
{
    //=======================================================================================
    //! Base class for model handlers that create models derived from WebMercatorModel.
    //! Instances of WebMercatorModel must be able to assume that their handler is-a WebMercatorModelHandler.
    //! Specifically, then will need to call the _CreateUrl method.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE WebMercator : Model
    {
        MODELHANDLER_DECLARE_MEMBERS ("WebMercatorModel", WebMercatorModel, WebMercator, Model, DGNPLATFORM_EXPORT)

    public:
        //! Create the URL to request the specified tile from a map service.
        //! @param[in] mapService       Identifies the source of the tiled map data. This is a token that is supplied by the
        //!                             subclass of WebMercatorModelHandler and stored on a WebMercatorModel instande in the DgnDb,
        //!                             in order to associate it with a map server. The WebMercatorModelHandler subclass looks at this string
        //!                             when constructing URLs at runtime for requesting tiles for the model.
        //! @param[in] mapType          Identifies the type of map data to request and display.
        //! @param[in] tileid           The location of the tile, according to the WebMercator tiling system
        DGNPLATFORM_EXPORT virtual BentleyStatus _CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorModel::Mercator const&, WebMercatorTilingSystem::TileId const& tileid) {return BSIERROR;}
    };
};
struct EXPORT_VTABLE_ATTRIBUTE StreetMapModel : WebMercatorModel
{
    DEFINE_T_SUPER(WebMercatorModel)
    StreetMapModel(CreateParams const& params) : T_Super(params) {}
};

namespace dgn_ModelHandler
{
    //=======================================================================================
    // A handler for models that communicate with one of the well known street map services
    // to obtain and display street maps and satellite imagery based on the WebMercator tiling system.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct StreetMap : WebMercator
    {
        MODELHANDLER_DECLARE_MEMBERS ("StreetMapModel", StreetMapModel, StreetMap, WebMercator, DGNPLATFORM_EXPORT)

        //! Identifies a well known street map tile service
        enum class MapService
            {
            MapQuest,               //!< MapQuest
            };

        //! The kind of map to display
        enum class MapType
            {
            Map,                    //!< Show a map
            SatelliteImage,         //!< Show a satellite image (if available)
            };

    protected:
        DGNPLATFORM_EXPORT virtual BentleyStatus _CreateUrl (Utf8StringR url, ImageUtilities::RgbImageInfo& imageInfo, WebMercatorModel::Mercator const&, WebMercatorTilingSystem::TileId const&) override;

        DGNPLATFORM_EXPORT Utf8String CreateMapquestUrl (WebMercatorTilingSystem::TileId const&, WebMercatorModel::Mercator const&);

    public:
        //! Create a new street map model in the DgnDb.
        //! @praam[in] db           The DgnDb
        //! @param[in] mapService   Identifies the map service that will supply the maps or imagery
        //! @param[in] mapType      Identifies the kind of map data to display
        //! @param[in] finerResolution If true, the external data model will download and display more and smaller tiles, if necessary, in order to get the best resolution.
        //!                              If false, fewer and larger tiles are obtained and displayed. That saves time but may sometimes result in slightly fuzzy resolution.
        //! @return the ID of the new external data model.
        DGNPLATFORM_EXPORT static DgnModelId CreateStreetMapModel(DgnDbR db, MapService mapService, MapType mapType, bool finerResolution);
    };
};

//=======================================================================================
// Displays a grid of latitude and longitude lines
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
#ifdef TBD_LATLNG_GRID
struct LatLongGridRealityDataHandler : PhysicalModel
{
protected:
    DGNPLATFORM_EXPORT virtual void _DrawView (ViewContextR) override;

};
#endif

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
