/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/WebMercator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/RealityDataCache.h>
#include <DgnPlatform/DgnViewport.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/ImageUtilities.h>

BEGIN_BENTLEY_DGN_NAMESPACE

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
        RgbImageInfo const* m_expectedImageInfo;
        RequestOptions() : m_expectedImageInfo(nullptr), RealityDataCacheOptions(false, false) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
        RequestOptions(RgbImageInfo const& expectedImageInfo) : m_expectedImageInfo(new RgbImageInfo(expectedImageInfo)), RealityDataCacheOptions(true, true) {DEFINE_BENTLEY_REF_COUNTED_MEMBER_INIT}
    public:
        ~RequestOptions() {DELETE_AND_CLEAR(m_expectedImageInfo);}
        static RefCountedPtr<RequestOptions> Create() {return new RequestOptions();}
        static RefCountedPtr<RequestOptions> Create(RgbImageInfo const& expectedImageInfo) {return new RequestOptions(expectedImageInfo);}
        RgbImageInfo const* GetExpectedImageInfo() const {return m_expectedImageInfo;}
    };

private:
    Utf8String      m_url;
    ByteStream      m_data;
    DateTime        m_creationDate;
    Utf8String      m_contentType;
    RgbImageInfo        m_rasterInfo;

private:
    static Utf8String SerializeRasterInfo(RgbImageInfo const&);
    static RgbImageInfo DeserializeRasterInfo(Utf8CP);

    TiledRaster(){}
        
protected:
    virtual Utf8CP _GetId() const override;
    virtual bool _IsExpired() const override;
    virtual BentleyStatus _InitFrom(IRealityDataBase const& self, RealityDataCacheOptions const&) override;
    virtual BentleyStatus _InitFrom(Utf8CP url, bmap<Utf8String, Utf8String> const& header, bvector<Byte> const& body, HttpRealityDataSource::RequestOptions const& options) override;
    virtual BentleyStatus _InitFrom(BeSQLite::Db& db, BeMutex& cs, Utf8CP key, BeSQLiteRealityDataStorage::SelectOptions const& options) override;
    virtual BentleyStatus _Persist(BeSQLite::Db& db, BeMutex& cs) const override;
    virtual BeSQLiteRealityDataStorage::DatabasePrepareAndCleanupHandlerPtr _GetDatabasePrepareAndCleanupHandler() const override;

public:
    static RefCountedPtr<TiledRaster> Create();    
    ByteStream const& GetData() const {return m_data;}
    DateTime GetCreationDate() const {return m_creationDate;}
    RgbImageInfo const& GetImageInfo() const {return m_rasterInfo;}
    Utf8String const& GetContentType() const {return m_contentType;}
};

//=======================================================================================
// Obtains and displays multi-resolution tiled raster reality data that is organized
// according to the WebMercator tiling system. Collaborates with WebMercatorModelHandler
// in order to send requests to a tile server.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

public:
    struct Mercator
        {
        AxisAlignedBox3d m_range;       //! The range covered by this map -- typically the project's extents -- could be the whole world
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

    //=======================================================================================
    //! Identifies a tile in the fixed WebMercator tiling system.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct TileId
    {
        uint8_t     zoomLevel;
        uint32_t    column;
        uint32_t    row;
    };


protected:
    Mercator m_mercator;

public:
    //! Create a new WebMercatorModel object, in preparation for loading it from the DgnDb.
    WebMercatorModel(CreateParams const& params) : T_Super(params) {}

    DGNPLATFORM_EXPORT void _AddTerrainGraphics(TerrainContextR) const override;
    DGNPLATFORM_EXPORT void _WriteJsonProperties(Json::Value&) const override;
    DGNPLATFORM_EXPORT void _ReadJsonProperties(Json::Value const&) override;
    AxisAlignedBox3d _QueryModelRange() const override {return m_mercator.m_range;}

    //! Create the URL to request the specified tile from a map service.
    //! @param[out] url             The returned URL
    //! @param[out] imageInfo       Expected image format
    //! @param[in] mapService       Identifies the type of map that is being displayed
    //! @param[in] tileid           The location of the tile, according to the WebMercator tiling system
    //! @return non-zero if URL cannot be computed
    virtual BentleyStatus _CreateUrl(Utf8StringR url, RgbImageInfo& imageInfo, TileId const& tileid) const {return BSIERROR;}

    virtual bool _ShouldRejectTile(TileId const& tileid, Utf8StringCR url, TiledRaster& realityData) const {return false;}

    //! Call this after creating a new model, in order to set up subclass-specific properties.
    void SetMercator(Mercator const&);
};

//=======================================================================================
// A street map model
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StreetMapModel : WebMercatorModel
{
    DGNMODEL_DECLARE_MEMBERS("StreetMapModel", WebMercatorModel);

    Utf8CP _GetCopyrightMessage() const override;
    BentleyStatus _CreateUrl(Utf8StringR url, RgbImageInfo& imageInfo, TileId const&) const override;
    bool _ShouldRejectTile(TileId const& tileid, Utf8StringCR url, TiledRaster& realityData) const override;

    Utf8String CreateOsmUrl(TileId const&) const;
    Utf8String CreateMapBoxUrl(TileId const&) const;
public:
    StreetMapModel(CreateParams const& params) : T_Super(params) {}
};

namespace dgn_ModelHandler
{
    //=======================================================================================
    //! Base class for model handlers that create models derived from WebMercatorModel.
    //! Instances of WebMercatorModel must be able to assume that their handler is-a WebMercatorModelHandler.
    //! Specifically, then will need to call the _CreateUrl method.
    // @bsiclass                                                    Sam.Wilson      10/2014
    //=======================================================================================
    struct EXPORT_VTABLE_ATTRIBUTE WebMercator : Spatial
    {
        MODELHANDLER_DECLARE_MEMBERS ("WebMercatorModel", WebMercatorModel, WebMercator, Spatial, DGNPLATFORM_EXPORT)
    };

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
#ifndef NDEBUG
            OpenStreetMaps = 999,         //!< OpenStreetMaps (developer builds only!)
#endif
            MapBox = 0
            };

        //! The kind of map to display
        enum class MapType
            {
            Map,                    //!< Show a map
            SatelliteImage,         //!< Show a satellite image (if available)
            };

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

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
