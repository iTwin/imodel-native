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
#include <DgnPlatform/TileTree.h>
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(MapTile)
DEFINE_POINTER_SUFFIX_TYPEDEFS(MapRoot)
DEFINE_REF_COUNTED_PTR(MapTile)
DEFINE_REF_COUNTED_PTR(MapRoot)

//=======================================================================================
//! Identifies a tile in the fixed WebMercator tiling system.
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct TileId
{
    uint8_t  m_zoomLevel;
    uint32_t m_row;
    uint32_t m_column;
    TileId() {}
    TileId(uint8_t zoomLevel, uint32_t col, uint32_t row){m_zoomLevel=zoomLevel; m_column=col; m_row=row;}
};

//=======================================================================================
//! The root of a multi-resolution web mercator map.
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct MapRoot : TileTree::Root
{
    ColorDef m_tileColor;                   //! for setting transparency
    uint8_t m_maxZoom;                      //! the maximum zoom level for this map
    uint32_t m_maxPixelSize;                //! the maximum size, in pixels, that a tile should stretched to. If the tile's size on screen is larger than this, use its children.
    Render::ImageSource::Format m_format;   //! the format of the tile image source
    Utf8String m_urlSuffix;                 //! any suffix to append to tile names. Usually includes key
    Transform m_mercatorToWorld;            //! linear transform from web mercator meters to world meters. Only used when reprojection fails.

    DPoint3d ToWorldPoint(GeoPoint);
    Utf8String _ConstructTileName(Dgn::TileTree::TileCR tile) const override;
    uint32_t GetMaxPixelSize() const {return m_maxPixelSize;}
    MapRoot(DgnDbR, TransformCR location, Utf8CP realityCacheName, Utf8StringCR rootUrl, Utf8StringCR urlSuffix, Dgn::Render::SystemP system, Render::ImageSource::Format, double transparency, 
            uint8_t maxZoom, uint32_t maxSize);
    ~MapRoot() {ClearAllTiles();}
};

//=======================================================================================
//! A web mercator map tile. May or may not have its graphics downloaded.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct MapTile : TileTree::Tile
{
    //=======================================================================================
    // @bsiclass                                                    Mathieu.Marchand  11/2016
    //=======================================================================================
    struct MapTileload : TileTree::TileLoad
        {
        MapTileload(Utf8StringCR url, TileTree::TileR tile, TileTree::TileLoadsPtr loads) :TileLoad(url, tile, loads, tile._GetTileName()) {}

        BentleyStatus _LoadTile() override;
        };

    TileId m_id;                                        //! tile id 
    Render::IGraphicBuilder::TileCorners m_corners;     //! 4 corners of tile, in world coordinates
    MapRootR m_mapRoot;                                 //! the MapRoot that loaded this tile.
    bool m_reprojected = false;                         //! if true, this tile has been correctly reprojected into world coordinates. Otherwise, it is not displayable.
    Render::GraphicPtr m_graphic;                       //! the texture for this tile.

    StatusInt ReprojectCorners(GeoPoint*);
    TileId GetTileId() const {return m_id;}
    MapTile(MapRootR mapRoot, TileId id, MapTileCP parent);

    bool TryLowerRes(TileTree::DrawArgsR args, int depth) const;
    void TryHigherRes(TileTree::DrawArgsR args) const;
    bool _HasChildren() const override;
    bool HasGraphics() const {return IsReady() && m_graphic.IsValid();}
    ChildTiles const* _GetChildren(bool load) const override;
    void _DrawGraphics(TileTree::DrawArgsR, int depth) const override;
    Utf8String _GetTileName() const override;
    double _GetMaximumSize() const override {return m_mapRoot.GetMaxPixelSize();}

    TileTree::TileLoadPtr _CreateTileLoad(TileTree::TileLoadsPtr) override;
};

//=======================================================================================
//! Obtains and displays multi-resolution tiled raster organized according to the WebMercator tiling system.
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
    Properties m_properties;
    mutable MapRootPtr m_root;
    void Load(Dgn::Render::SystemP) const;
    virtual Utf8String _GetRootUrl() const {return "";}
    virtual Utf8String _GetUrlSuffix() const {return "";}

public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(WebMercatorModel::T_Super::CreateParams);
    public:
        Properties m_properties;
        DGNPLATFORM_EXPORT CreateParams(DgnDbR dgndb, DgnElementId modeledElementId, Properties const& props);
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    };

    //! Create a new WebMercatorModel object
    WebMercatorModel(CreateParams const& params) : T_Super(params), m_properties(params.m_properties) {}

    void _AddTerrainGraphics(TerrainContextR) const override;
    void _WriteJsonProperties(Json::Value&) const override;
    void _ReadJsonProperties(Json::Value const&) override;
    double GetGroundBias() const {return m_properties.m_groundBias;}

    //! Call this after creating a new model, to set up properties.
    void SetProperties(Properties const& props) {m_properties=props;}

    Properties const& GetProperties() const {return m_properties;}
    Properties& GetPropertiesR() {return m_properties;}
};

//=======================================================================================
//! A street map model
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StreetMapModel : WebMercatorModel
{
    DGNMODEL_DECLARE_MEMBERS("StreetMapModel", WebMercatorModel);

    Utf8CP _GetCopyrightMessage() const override;
    Utf8String _GetRootUrl() const override;
    Utf8String _GetUrlSuffix() const override;

public:
    StreetMapModel(CreateParams const& params) : T_Super(params) {}

    static Utf8CP GetMapboxStreetsUrl() {return "http://api.mapbox.com/v4/mapbox.streets/";}
    static Utf8CP GetMapboxSatelliteUrl() {return "http://api.mapbox.com/v4/mapbox.satellite/";}
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
