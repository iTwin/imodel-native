/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/WebMercator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(WebMercatorModel)
DEFINE_POINTER_SUFFIX_TYPEDEFS(WebMercatorDisplay)

DEFINE_REF_COUNTED_PTR(MapTile)
DEFINE_REF_COUNTED_PTR(MapRoot)
DEFINE_REF_COUNTED_PTR(WebMercatorDisplay)

enum class MapType : int {None=0, Street=1, Aerial=2, Hybrid=3};

//=======================================================================================
// Interface between QuadTree implementation and the providers of the imagery.
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct ImageryProvider : RefCountedBase
{
    virtual ~ImageryProvider () {}

    enum class TemplateUrlLoadStatus : int {NotFetched=0, Requested=1, Received=2, Failed=3, Abandoned=4};

    // Imagery Providery implementation-specific methods.

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    virtual Utf8String _GetProviderName() const = 0;

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

    virtual bool _MatchesMissingTile (ByteStream& tileBytes) const {return false;}

    // Gets the message to be displayed to credit provider(s).
    virtual Utf8String _GetCopyrightMessage(ViewController& viewController) const = 0;

    virtual Render::RgbaSpriteP _GetCopyrightSprite () { return nullptr; }


    virtual MapType _GetMapType () const = 0;

    // if the provider wants to set a maximum duration for tiles, set it as a duration in milliseconds.
    // if not relevant, return 0. Bing Maps uses this to enforce a 3 day limit for tile life.
    virtual uint64_t _GetMaxValidDuration () const { return 0; }

    // Some imagery providers (such as Bing) need to see the selected tiles so the copyright message can be computed.
    virtual void _OnSelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const {}

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

    Render::RgbaSpritePtr m_copyrightSprite;

    DPoint3d ToWorldPoint(GeoPoint);
    Utf8String _ConstructTileResource(TileTree::TileCR tile) const override;
    Utf8CP _GetName() const override {return "WebMercator";}
    bool _IsBackgroundImagery() const override {return true;}

    MapRoot(DgnDbR db, DgnModelId modelId, TransformCR location, ImageryProviderR imageryProvider, Dgn::Render::SystemP system, Render::ImageSource::Format, double transparency, uint32_t maxSize);
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
        Loader(Utf8StringCR url, TileTree::TileR tile, TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) : TileTree::TileLoader(url, tile, loads, tile._GetTileCacheKey(), renderSys) {}
        uint64_t      _GetMaxValidDuration() const override;
        BentleyStatus _LoadTile() override;
    };

    bool m_reprojected = false;  //! if true, this tile has been correctly reprojected into world coordinates. Otherwise, it is not displayable.
    StatusInt ReprojectCorners(GeoPoint*);
    MapTile(MapRootR mapRoot, TileTree::QuadTree::TileId id, MapTileCP parent);
    void _DrawGraphics(TileTree::DrawArgsR) const override;
    TileTree::TilePtr _CreateChild(TileTree::QuadTree::TileId id) const override {return new MapTile(GetMapRoot(), id, this);}
    MapRoot& GetMapRoot() const {return (MapRoot&) m_root;}
    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys = nullptr) override {return new Loader(GetRoot()._ConstructTileResource(*this), *this, loads, renderSys);}
    double _GetMaximumSize() const override {return 0 == GetDepth() ? 0.0 : T_Super::_GetMaximumSize();}
    TileTree::Tile::SelectParent _SelectTiles(bvector<TileTree::TileCPtr>&, TileTree::DrawArgsR) const override;
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
    TileTree::RootPtr LoadTileTree(Dgn::Render::SystemP) const;

    double m_groundBias;
    double m_transparency;
    ImageryProviderPtr m_provider;

    void FromJson(Json::Value const& value);
    void ToJson(Json::Value& value) const;

public:
    BE_JSON_NAME(providerName);
    BE_JSON_NAME(groundBias);
    BE_JSON_NAME(transparency);
    BE_JSON_NAME(mapType)       // the mapType in the Json is actually processed by the MapProviders.
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
    TileTree::RootPtr _GetTileTree(RenderContextR) override;
    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;
    double GetGroundBias() const {return m_groundBias;}
    Utf8String _GetCopyrightMessage(ViewController&) const override;
    Render::RgbaSpriteP _GetCopyrightSprite(ViewController&) const override;
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
private:
    Utf8String m_baseUrl;
    MapType m_mapType = MapType::Street;

    // constructor used prior to specifying from stored Json values.
    MapBoxImageryProvider() {}

public:
    BE_PROP_NAME(MapBoxProvider)

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_MapBoxProvider();}

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCopyrightMessage(ViewController& viewController) const override;

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMaximumZoomLevel(bool forPrinting) override {return 19;}

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    Utf8String _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const override;
    
    void _FromJson(Json::Value const& value) override;

    void _ToJson(Json::Value&) const override;

    MapType _GetMapType () const override {return m_mapType; }

    static MapBoxImageryProvider* Create (Json::Value const& providerDataValue);
};

struct BingImageryProvider;
DEFINE_REF_COUNTED_PTR(BingImageryProvider)

//=======================================================================================
// Bing Imagery Provider
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct BingImageryProvider : ImageryProvider
{
    struct Coverage
        {
        double                      m_lowerLeftLongitude;
        double                      m_lowerLeftLatitude;
        double                      m_upperRightLongitude;
        double                      m_upperRightLatitude;
        uint8_t                     m_minimumZoomLevel;
        uint8_t                     m_maximumZoomLevel;
        };

    struct Attribution
        {
        Utf8String                  m_copyrightMessage;
        std::vector<Coverage>       m_coverageList;
        };

private:
    Utf8String m_urlTemplate;
    Utf8String m_creditUrl;
    MapType m_mapType = MapType::Street;
    uint8_t m_maximumZoomLevel = 19;
    uint8_t m_minimumZoomLevel = 0;
    int m_tileWidth = 256;
    int m_tileHeight = 256;
    BeAtomic<TemplateUrlLoadStatus> m_templateUrlLoadStatus;
    
    std::vector<Attribution>        m_attributions;

    BeAtomic<bool>                  m_logoValid;
    ByteStream                      m_logoByteStream;
    Utf8String                      m_logoContentType;
    Render::RgbaSpritePtr           m_copyrightSprite;

    BeAtomic<uint32_t>              m_missingTileDataSize;
    uint8_t*                        m_missingTileData;  // one third of the data we get back, taken from the middle.

    static BingImageryProviderPtr   s_streetMapProvider;
    static BingImageryProviderPtr   s_aerialMapProvider;
    static BingImageryProviderPtr   s_hybridMapProvider;

    // constructor used prior to specifying from stored Json values.
    void                ReadAttributionsFromJson (Json::Value const& response);

    // constructor used prior to specifying from stored Json values.
    BingImageryProvider ()
        {
        m_templateUrlLoadStatus.store (TemplateUrlLoadStatus::NotFetched); 
        m_missingTileDataSize.store (0);
        m_missingTileData = nullptr;
        m_logoValid.store (false);
        }

    virtual ~BingImageryProvider()
        {
        if (nullptr != m_missingTileData)
            free (m_missingTileData);
        }

public:
    BE_PROP_NAME(BingProvider)

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_BingProvider();}

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

    folly::Future<TemplateUrlLoadStatus> _FetchTemplateUrl() override;

    TemplateUrlLoadStatus _GetTemplateUrlLoadStatus() const override {return m_templateUrlLoadStatus;}

    bool _MatchesMissingTile (ByteStream& tileBytes) const override;

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCopyrightMessage(ViewController& viewController) const override;

    Render::RgbaSpriteP _GetCopyrightSprite () override;

    uint64_t _GetMaxValidDuration () const override;

    MapType _GetMapType () const override {return m_mapType; }

    void _OnSelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const override;

    static BingImageryProvider* Create (Json::Value const& providerDataValue);
    };

DEFINE_REF_COUNTED_PTR(BingImageryProvider)

//=======================================================================================
// Here Imagery Provider (Here is the successor to NavTeq).
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct HereImageryProvider : ImageryProvider
{
private:
    Utf8String m_urlTemplate;
    Utf8String m_appId;
    Utf8String m_appCode;
    MapType m_mapType = MapType::Street;
    uint8_t m_maximumZoomLevel = 21;
    int m_tileWidth = 256;
    int m_tileHeight = 256;

public:
    BE_PROP_NAME(HereProvider)

    // constructor used prior to specifying from stored Json values.
    HereImageryProvider();

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_HereProvider();}

    // Gets the message to be displayed to credit provider(s). 
    Utf8String _GetCopyrightMessage(ViewController& viewController) const override;

    // Gets the maximum zoom level alllowed (provider dependent)
    uint8_t _GetMaximumZoomLevel(bool forPrinting) override {return m_maximumZoomLevel;}

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    Utf8String _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    Utf8String _ConstructUrl(TileTree::QuadTree::Tile const& tile) const override;
    
    void _FromJson(Json::Value const& value) override;

    void _ToJson(Json::Value&) const override;

    MapType _GetMapType () const override {return m_mapType; }

    static HereImageryProvider* Create (Json::Value const& providerDataValue);
};

//=======================================================================================
// @bsiclass                                                    Ray.Bentley     10/2018
//=======================================================================================
struct WebMercatorDisplayHandler : DisplayStyle::BackgroundMapDisplayHandler
{
    ImageryProviderPtr  m_provider;
    Json::Value         m_settings;

    WebMercatorDisplayHandler(Json::Value const&  settings);
    void _Initialize(Json::Value const& settings) override;

    TileTree::RootPtr _GetTileTree(SceneContextR sceneContext) override;
};


}; // end WebMercator namespace


//! @endGroup

END_BENTLEY_DGN_NAMESPACE
