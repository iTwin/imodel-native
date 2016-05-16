/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/WebMercator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnViewport.h>
#include <DgnPlatform/DgnDbTables.h>
#include <DgnPlatform/ImageUtilities.h>
#include <algorithm>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace WebMercator
{

//=======================================================================================
//! Identifies a tile in the fixed WebMercator tiling system.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct TileId
{
    uint8_t  m_zoomLevel;
    uint32_t m_column;
    uint32_t m_row;
    TileId() {}
    TileId(uint8_t zoomLevel, uint32_t col, uint32_t row){m_zoomLevel=zoomLevel; m_column=col; m_row=row;}
    bool operator<(TileId const& rhs) const;
};

//=======================================================================================
// A cached tile. May or may not be loaded.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct Tile : RefCountedBase, NonCopyableClass
{
    struct Corners
    {
        DPoint3d m_pts[4];
    };

    static uint64_t Next();
    enum LoadStatus {NotLoaded=0, Queued=1, Loading=2, Ready=3, NotFound=4, Abandoned=5};

    mutable uint64_t m_lastAccessed;
    BeAtomic<int> m_loadStatus;
    TileId m_id;
    Corners m_corners;
    Render::GraphicPtr m_graphic;

    void Accessed() const {m_lastAccessed = Next();}
    bool IsLoaded() const {return LoadStatus::Ready == m_loadStatus.load();}
    void SetQueued() {return m_loadStatus.store(LoadStatus::Queued);}
    void SetLoaded() {return m_loadStatus.store(LoadStatus::Ready);}
    void SetNotFound() {BeAssert(false); return m_loadStatus.store(LoadStatus::NotFound);}
    bool IsQueued() const {return m_loadStatus.load() == LoadStatus::Queued;}
    TileId GetTileId() const {return m_id;}
    Tile(TileId id, Corners const& corners) : m_id(id), m_corners(corners){Accessed();}
};

DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile)
DEFINE_REF_COUNTED_PTR(Tile)

//=======================================================================================
// Obtains and displays multi-resolution tiled raster that is organized
// according to the WebMercator tiling system. 
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : SpatialModel
{
    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

public:
    struct Properties
        {
        Utf8String m_mapService;        //! Identifies the source of the tiled map data. This is a token that is supplied by the 
                                        //! subclass of WebMercatorModel and stored in the DgnDb to associate it with a map server. 
                                        //! The WebMercatorModel subclass uses this string when constructing URLs at runtime for requesting tiles.
        Utf8String m_mapType;           //! Identifies the type of map data to request and display.
        bool m_finerResolution;         //! true => download and display more and smaller tiles, if necessary, in order to get the best resolution.
                                        //! false => download and display fewer and larger tiles, resulting in sometimes slightly fuzzy resolution.
        double m_groundBias=-1.0;       //! An offset from the ground plane to draw map. By default, draw map 1 meter below sea level (negative values are below sea level)
        double m_transparency=0.0;      //! 0=fully opaque, 1.0=fully transparent

        void SetGroundBias(double val) {m_groundBias=val;}
        double GetGroundBias() const {return m_groundBias;}
        void SetTransparency(double val) {m_transparency=std::max(0.0, std::min(val, .9));} // limit range bewteen 0 and .9
        double GetTransparency() const {return m_transparency;}
        void ToJson(Json::Value&) const;
        void FromJson(Json::Value const&);
        };

protected:
    struct TileCache
    {
    private:
        int m_maxSize = 200;
        bmap<TileId, TilePtr> m_map;
        void Trim();

    public:
        void Clear() {m_map.clear();}
        void Add(TileId id, Tile* tile){Trim(); m_map.Insert(id,tile);}
        TilePtr Get(TileId id) {auto ifound = m_map.find(id); return (ifound == m_map.end()) ? nullptr : ifound->second;}
    };

    Properties m_properties;
    mutable RealityDataCachePtr m_realityDataCache;
    mutable TileCache m_tileCache;

public:
    void RequestTile(TileId, TileR, Render::SystemR) const;
    TilePtr CreateTile(TileId id, Tile::Corners const&, Render::SystemR) const;
        
    RealityDataCache& GetRealityDataCache() const;
    TileCache& GetTileCache() const {return m_tileCache;}

    //! Create a new WebMercatorModel object, in preparation for loading it from the DgnDb.
    WebMercatorModel(CreateParams const& params) : T_Super(params) {}

    void _AddTerrainGraphics(TerrainContextR) const override;
    void _WriteJsonProperties(Json::Value&) const override;
    void _ReadJsonProperties(Json::Value const&) override;
    double GetGroundBias() const {return m_properties.m_groundBias;}

    //! Create the URL to request the specified tile from a map service.
    //! @param[out] url The returned URL
    //! @param[in] tileid  The location of the tile, according to the WebMercator tiling system
    //! @return SUCCESS if URL was computed and is valid
    virtual BentleyStatus _CreateUrl(Utf8StringR url, TileId tileid) const {return ERROR;}

    virtual bool _ShouldRejectTile(TileId tileid, Utf8StringCR url, ByteStream const& data) const {return false;}

    //! Call this after creating a new model, to set up properties.
    void SetProperties(Properties const& props) {m_properties=props;}

    Properties const& GetProperties() {return m_properties;}
    Properties& GetPropertiesR() {return m_properties;}

    void ClearTileCache() {m_tileCache.Clear();}
    DGNPLATFORM_EXPORT BentleyStatus DeleteCacheFile(); //! delete the local SQLite file holding the cache of downloaded tiles.
};

//=======================================================================================
// A street map model
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StreetMapModel : WebMercatorModel
{
    DGNMODEL_DECLARE_MEMBERS("StreetMapModel", WebMercatorModel);

    Utf8CP _GetCopyrightMessage() const override;
    BentleyStatus _CreateUrl(Utf8StringR url, TileId) const override;
    bool _ShouldRejectTile(TileId tileid, Utf8StringCR url, ByteStream const& realityData) const override;

    Utf8String CreateOsmUrl(TileId) const;
    Utf8String CreateMapBoxUrl(TileId) const;
public:
    StreetMapModel(CreateParams const& params) : T_Super(params) {}
};

//=======================================================================================
//! Base class for model handlers that create models derived from WebMercatorModel.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ModelHandler : dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("WebMercatorModel", WebMercatorModel, ModelHandler, dgn_ModelHandler::Spatial, DGNPLATFORM_EXPORT)
};

//=======================================================================================
// A handler for models that communicate with one of the well known street map services
// to obtain and display street maps and satellite imagery based on the WebMercator tiling system.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct StreetMapHandler : ModelHandler
{
    MODELHANDLER_DECLARE_MEMBERS ("StreetMapModel", StreetMapModel, StreetMapHandler, ModelHandler, DGNPLATFORM_EXPORT)

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
    //! @param[in] db           The DgnDb
    //! @param[in] mapService   Identifies the map service that will supply the maps or imagery
    //! @param[in] mapType      Identifies the kind of map data to display
    //! @param[in] finerResolution If true, the external data model will download and display more and smaller tiles, if necessary, in order to get the best resolution.
    //!                            If false, fewer and larger tiles are obtained and displayed. That saves time but may sometimes result in slightly fuzzy resolution.
    //! @return the Id of the new external data model.
    DGNPLATFORM_EXPORT static DgnModelId CreateStreetMapModel(DgnDbR db, MapService mapService, MapType mapType, bool finerResolution);
};

}; // end WebMercator namespace

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
