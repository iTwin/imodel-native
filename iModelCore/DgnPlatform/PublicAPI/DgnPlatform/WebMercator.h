/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/WebMercator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
// Interface between QuadTree implementation and the providers of the imagery.
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct ImageryProvider : RefCountedBase
{
    enum class TemplateUrlLoadStatus : int {NotFetched=0, Requested=1, Received=2, Failed=3, Abandoned=4};

    // Imagery Providery implementation-specific methods.

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    virtual Utf8String _GetProviderName() const = 0;

    // Gets the message to be displayed to credit provider(s).
    virtual Utf8String _GetCreditMessage() const = 0;

    // Gets an URL for the image to be displayed to credit the provider.
    virtual Utf8String _GetCreditUrl() const = 0;

    // Gets the tile width (usually 256)
    virtual int _GetTileWidth() {return 256;}

    // Gets the tile height (usually 256)
    virtual int _GetTileHeight() {return 256;}

    // Gets the minimum zoom level (usually 0)
    virtual uint8_t _GetMinimumZoomLevel() {return 0;}

    // Gets the maximum zoom level alllowed (provider dependent)
    virtual uint8_t _GetMaximumZoomLevel(bool forPrinting) = 0;

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    virtual Utf8String _GetCacheFileName() const = 0;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    virtual Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const = 0;
    
    // Gets the Json that is saved to the model and used to reconstruct the ImageryProvider in subsequent sessions.
    virtual void _ToJson(Json::Value&) const = 0;

    // Reconstructs the ImageryProvider parameters from the Json that is saved to the model.
    virtual void _FromJson(Json::Value const& value) = 0;

    // Retrieve the Tile Template Url from the service.
    virtual folly::Future<TemplateUrlLoadStatus> _FetchTemplateUrl() {return TemplateUrlLoadStatus::Received;}

    // if a Tile Template Url must be retrieved, return the current status. If none is needed, return "Received"
    virtual TemplateUrlLoadStatus _GetTemplateUrlLoadStatus() const {return TemplateUrlLoadStatus::Received;}

    // sets the Tile Template Url retrieval status.
    virtual void _SetTemplateUrlLoadStatus(TemplateUrlLoadStatus) {}
};

DEFINE_REF_COUNTED_PTR(ImageryProvider)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ImageryProvider)

//=======================================================================================
//! The root of a multi-resolution web mercator map.
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct MapRoot : TileTree::QuadTree::Root
{
    Render::ImageSource::Format m_format; //! the format of the tile image source
    Transform m_mercatorToWorld;  //! linear transform from web mercator meters to world meters. Only used when reprojection fails.
    ImageryProviderPtr m_imageryProvider; //! procures the image tiles from the source tile server.

    DPoint3d ToWorldPoint(GeoPoint);
    Utf8String _ConstructTileResource(TileTree::TileCR tile) const override;
    Utf8CP _GetName() const override {return "WebMercator";}
    MapRoot(DgnDbR, TransformCR location, ImageryProviderR imageryProvider, Dgn::Render::SystemP system, Render::ImageSource::Format, double transparency, uint32_t maxSize);
    ~MapRoot() {ClearAllTiles();}
};

//=======================================================================================
//! A web mercator map tile. May or may not have its graphics downloaded.
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct MapTile : TileTree::QuadTree::Tile
{
    DEFINE_T_SUPER(TileTree::QuadTree::Tile)

    struct Loader : TileTree::TileLoader
    {
        Loader(Utf8StringCR url, TileTree::TileR tile, TileTree::TileLoadStatePtr loads) : TileTree::TileLoader(url, tile, loads, tile._GetTileCacheKey()) {}
        BentleyStatus _LoadTile() override;
    };

    bool m_reprojected = false;  //! if true, this tile has been correctly reprojected into world coordinates. Otherwise, it is not displayable.
    StatusInt ReprojectCorners(GeoPoint*);
    MapTile(MapRootR mapRoot, TileTree::QuadTree::TileId id, MapTileCP parent);
    void _DrawGraphics(TileTree::DrawArgsR) const override;
    TileTree::TilePtr _CreateChild(TileTree::QuadTree::TileId id) const override {return new MapTile(GetMapRoot(), id, this);}
    MapRoot& GetMapRoot() const {return (MapRoot&) m_root;}
    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads) override {return new Loader(GetRoot()._ConstructTileResource(*this), *this, loads);}
};

//=======================================================================================
//! Obtains and displays multi-resolution tiled raster organized according to the WebMercator tiling system.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : SpatialModel
{
    friend struct FetchTemplateUrlProgressiveTask;

    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

protected:
    TileTree::RootPtr Load(Dgn::Render::SystemP) const;

    double m_groundBias;
    double m_transparency;
    ImageryProviderPtr m_provider;

    void FromJson(Json::Value const& value);
    void ToJson(Json::Value& value) const;

public:
    BE_JSON_NAME(providerName);
    BE_JSON_NAME(groundBias);
    BE_JSON_NAME(transparency);
    BE_JSON_NAME(providerData); // identifier of ProviderData subfolder

    struct CreateParams : T_Super::CreateParams
    {
        friend WebMercatorModel;
        DEFINE_T_SUPER(WebMercatorModel::T_Super::CreateParams);

    private:
        Json::Value m_jsonParameters;

    public:
        // used when creating a new WebMercatorModel from user inputs, which are passed in the jsonParameters.
        CreateParams(DgnDbR dgndb, DgnElementId modeledElementId, JsonValueCR jsonParameters) :
                T_Super::CreateParams(dgndb, DgnClassId(dgndb.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_WebMercatorModel)), modeledElementId),
                m_jsonParameters(jsonParameters) {}

        // used when creating model from existing DgnDb.
        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    };

    //! Create a new WebMercatorModel from ModelHandler::CreateWebMercatorModel method. The caller sets up the ImageryProvider from user input.
    DGNPLATFORM_EXPORT WebMercatorModel(CreateParams const& params);

    TileTree::RootPtr _CreateTileTree(Render::SystemP) override;
    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;
    double GetGroundBias() const {return m_groundBias;}
    Utf8String _GetCopyrightMessage() const override;
};

DEFINE_REF_COUNTED_PTR(WebMercatorModel)

//=======================================================================================
//! Base class for model handlers that create models derived from WebMercatorModel.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ModelHandler : dgn_ModelHandler::Spatial
{
    MODELHANDLER_DECLARE_MEMBERS ("WebMercatorModel", WebMercatorModel, ModelHandler, dgn_ModelHandler::Spatial, DGNPLATFORM_EXPORT)
};

//=======================================================================================
// MapBox Imagery Provider
// This is the default, free provider. The same Bentley-provided key is used by all users.
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct MapBoxImageryProvider : ImageryProvider
{
    // the map types available from MapBox
    enum class MapType
    {
        StreetMap = 0,
        Satellite = 1,
        StreetsAndSatellite = 2,
    };

private:
    Utf8String m_baseUrl;
    MapType m_mapType = MapType::StreetMap;

public:
    BE_JSON_NAME(mapType)
    BE_PROP_NAME(MapBoxProvider)

    // constructor used prior to specifying from stored Json values.
    MapBoxImageryProvider() {}

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_MapBoxProvider();}

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCreditMessage() const override;

    // Gets an URL for the image to be displayed to credit the provider.
    Utf8String _GetCreditUrl() const override;

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMaximumZoomLevel(bool forPrinting) override {return 19;}

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    Utf8String _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const override;
    
    void _FromJson(Json::Value const& value) override;

    void _ToJson(Json::Value&) const override;
};

//=======================================================================================
// Bing Imagery Provider
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct BingImageryProvider : ImageryProvider
{
    // the map types available from Bing
    enum class MapType
    {
        Road = 0,
        Aerial = 1,
        AerialWithLabels = 2,
    };

private:
    Utf8String m_urlTemplate;
    Utf8String m_creditUrl;
    MapType m_mapType = MapType::Road;
    uint8_t m_maximumZoomLevel = 19;
    uint8_t m_minimumZoomLevel = 0;
    int m_tileWidth = 256;
    int m_tileHeight = 256;
    BeAtomic<TemplateUrlLoadStatus> m_templateUrlLoadStatus;

public:
    BE_PROP_NAME(BingProvider)
    BE_JSON_NAME(mapType)

    // constructor used prior to specifying from stored Json values.
    BingImageryProvider() {m_templateUrlLoadStatus.store(TemplateUrlLoadStatus::NotFetched);}

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_BingProvider();}

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCreditMessage() const override;

    // Gets an URL for the image to be displayed to credit the provider.
    Utf8String _GetCreditUrl() const override;

    // Gets the tile width (usually 256)
    int _GetTileWidth() override {return m_tileWidth;}

    // Gets the tile height (usually 256)
    int _GetTileHeight() override {return m_tileHeight;}

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMaximumZoomLevel(bool forPrinting) override {return m_maximumZoomLevel;}

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMinimumZoomLevel() override {return m_minimumZoomLevel;}

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    Utf8String _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const override;
    
    void _FromJson(Json::Value const& value) override;

    void _ToJson(Json::Value&) const override;

    TemplateUrlLoadStatus _GetTemplateUrlLoadStatus() const override {return m_templateUrlLoadStatus;}

    void _SetTemplateUrlLoadStatus(TemplateUrlLoadStatus status) override {m_templateUrlLoadStatus.store(status);}

    folly::Future<TemplateUrlLoadStatus> _FetchTemplateUrl() override;
    };

DEFINE_REF_COUNTED_PTR(BingImageryProvider)

//=======================================================================================
// Here Imagery Provider (Here is the successor to NavTeq).
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct HereImageryProvider : ImageryProvider
{
    enum class MapType
    {
        // Note: Here provides a big assortment of different types, I picked these to simplify it.
        Map      = 0,
        Aerial   = 1,
        Combined = 2,
    };

private:
    Utf8String m_urlTemplate;
    Utf8String m_creditUrl;
    Utf8String m_appId;
    Utf8String m_appCode;
    MapType m_mapType = MapType::Map;
    uint8_t m_minimumZoomLevel = 0;
    uint8_t m_maximumZoomLevel = 21;
    int m_tileWidth = 256;
    int m_tileHeight = 256;

public:
    BE_PROP_NAME(HereProvider)
    BE_JSON_NAME(mapType)

    // constructor used prior to specifying from stored Json values.
    HereImageryProvider();

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_HereProvider();}

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCreditMessage() const override;

    // Gets an URL for the image to be displayed to credit the provider.
    Utf8String _GetCreditUrl() const override;

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMaximumZoomLevel(bool forPrinting) override {return m_maximumZoomLevel;}

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMinimumZoomLevel() override {return m_minimumZoomLevel;}

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    Utf8String _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const override;
    
    void _FromJson(Json::Value const& value) override;

    void _ToJson(Json::Value&) const override;
    };

}; // end WebMercator namespace

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
