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
#include <DgnPlatform/RealityDataCache.h>
#include <algorithm>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
    bool IsAbandoned() const {return LoadStatus::Abandoned== m_loadStatus.load();}
    void SetQueued() {m_loadStatus.store(LoadStatus::Queued);}
    void SetLoaded() {m_loadStatus.store(LoadStatus::Ready);}
    void SetAbandoned() {m_loadStatus.store(LoadStatus::Abandoned);}
    void SetNotFound() {m_loadStatus.store(LoadStatus::NotFound);}
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
        //! Identifies a well known street map tile service
        enum class MapService
        {
            MapBox = 0,
        };

        //! The kind of map to display
        enum class MapType
        {
            Map,            //!< Show a map
            Satellite,      //!< Show a satellite image (if available)
        };

        MapService m_mapService=MapService::MapBox;  //! Identifies the source of the tiled map data.
        MapType m_mapType=MapType::Map;              //! Identifies the type of tiles to request and display.
        bool m_finerResolution=false;   //! true => download and display more and smaller tiles, if necessary, in order to get the best resolution.
        double m_groundBias=-1.0;       //! An offset from the ground plane to draw map. By default, draw map 1 meter below sea level (negative values are below sea level)
        double m_transparency=0.0;      //! 0=fully opaque, 1.0=fully transparent

        void SetMapType(MapType val) {m_mapType=val;}
        void SetGroundBias(double val) {m_groundBias=val;}
        double GetGroundBias() const {return m_groundBias;}
        void SetTransparency(double val) {m_transparency=std::max(0.0, std::min(val, .9));} // limit range bewteen 0 and .9
        double GetTransparency() const {return m_transparency;}
        bool IsTransparent() const {return 0.0 < m_transparency;}
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
        int GetMaxSize() const {return m_maxSize;}
        void SetMaxSize(int val) {m_maxSize=val;}
        void Clear() {m_map.clear();}
        void Add(TileId id, Tile* tile){Trim(); m_map.Insert(id,tile);}
        TilePtr Get(TileId id) {auto ifound = m_map.find(id); return (ifound == m_map.end()) ? nullptr : ifound->second;}
    };

    Properties m_properties;
    Dgn::RealityData::Cache2Ptr m_cache;
    mutable TileCache m_tileCache;
    void CreateCache();

public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(WebMercatorModel::T_Super::CreateParams);
    public:
        Properties m_properties;
        DGNPLATFORM_EXPORT CreateParams(DgnDbR dgndb, Properties const& props);
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    };

    void RequestTile(TileId, TileR, Render::SystemR) const;
    TilePtr CreateTile(TileId id, Tile::Corners const&, Render::SystemR) const;

    TileCache& GetTileCache() const {return m_tileCache;}

    //! Create a new WebMercatorModel object, in preparation for loading it from the DgnDb.
    WebMercatorModel(CreateParams const& params) : T_Super(params), m_properties(params.m_properties) {CreateCache();}

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

    Properties const& GetProperties() const {return m_properties;}
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

    //! Create a new street map model in the DgnDb.
    //! @param[in] params       The parameters for the new model
    //! @return the Id of the new StreetMap Model.
    DGNPLATFORM_EXPORT static DgnModelId CreateStreetMapModel(StreetMapModel::CreateParams const& params);
};

}; // end WebMercator namespace

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
