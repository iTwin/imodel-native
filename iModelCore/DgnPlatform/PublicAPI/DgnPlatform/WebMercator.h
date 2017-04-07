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
struct  ImageryProvider : RefCountedBase
{
    // Imagery Providery implementation-specific methods.

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    virtual Utf8CP      _GetProviderName() const = 0;

    // Gets the message to be displayed to credit provider(s).
    virtual Utf8CP      _GetCreditMessage() const = 0;

    // Gets an URL for the image to be displayed to credit the provider.
    virtual Utf8CP      _GetCreditUrl() const = 0;

    // Gets the tile width (usually 256)
    virtual int         _GetTileWidth () { return 256; }

    // Gets the tile height (usually 256)
    virtual int         _GetTileHeight () { return 256; }

    // Gets the minimum zoom level (usually 0)
    virtual int         _GetMinimumZoomLevel () { return 0; }

    // Gets the maximum zoom level alllowed (provider dependent)
    virtual int         _GetMaximumZoomLevel (bool forPrinting) = 0;

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    virtual Utf8CP      _GetCacheFileName() const = 0;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    virtual Utf8String  _ConstructUrl (TileTree::QuadTree::Tile const& tile)  const = 0;
    
    // Gets the Json that is saved to the model and used to reconstruct the ImageryProvider in subsequent sessions.
    virtual void        _ToJson (Json::Value&) const = 0;

    // Reconstructs the ImageryProvider parameters from the Json that is saved to the model.
    virtual void        _FromJson (Json::Value const& value) = 0;

};

DEFINE_REF_COUNTED_PTR(ImageryProvider)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ImageryProvider)


//=======================================================================================
//! The root of a multi-resolution web mercator map.
// @bsiclass                                                    Keith.Bentley   08/16
//=======================================================================================
struct MapRoot : TileTree::QuadTree::Root
{
    Render::ImageSource::Format m_format;   //! the format of the tile image source
    Transform                   m_mercatorToWorld;            //! linear transform from web mercator meters to world meters. Only used when reprojection fails.
    ImageryProviderPtr          m_imageryProvider;

    DPoint3d ToWorldPoint(GeoPoint);
    Utf8String _ConstructTileResource (TileTree::TileCR tile) const override;
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
    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

public:
protected:
    TileTree::RootPtr Load(Dgn::Render::SystemP) const;

    ImageryProviderPtr      m_provider;
    double                  m_groundBias;
    double                  m_transparency;
    mutable MapRootPtr      m_root;

    void FromJson(Json::Value const& value);
    void ToJson(Json::Value& value) const;

public:
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(WebMercatorModel::T_Super::CreateParams);

        // properties common to all ImageryProviders
        Utf8String          m_providerName;
        double              m_groundBias   = -1.0;
        double              m_transparency = 0.0;
        Json::Value         m_providerParameters;

        public:

        // Gets the ImageryProvider name - the ImagerProvider is constructed if the name matches those in the list of available providers.
        Utf8String          GetProviderName () { return m_providerName; }

        // Sets the ImageryProvider name.
        void                SetProviderName (Utf8CP providerName) { m_providerName.assign (providerName); }

        JsonValueCR         GetProviderParameters () { return m_providerParameters; }

        // The offset from ground level at which the imagery is displayed
        double      GetGroundBias () { return m_groundBias; }

        // The offset from ground level at which the imagery is displayed
        void        SetGroundBias (double value) { m_groundBias = value; }

        // The transparency at which the imagery is displayed
        double      GetTransparency () { return m_transparency; }

        // The transparency at which the imagery is displayed
        void        SetTransparency (double value) { m_transparency = value; }

        // used when creating a new WebMercatorModel from user inputs.
        CreateParams(DgnDbR dgndb, DgnElementId modeledElementId, double groundBias, double transparency, Utf8CP providerName, JsonValueCR providerParameters) :
                T_Super::CreateParams(dgndb, DgnClassId(dgndb.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_WebMercatorModel)), modeledElementId),
                m_groundBias (groundBias), m_transparency(transparency), m_providerName (providerName), m_providerParameters (providerParameters) {}

        CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    };

    //! Create a new WebMercatorModel from ModelHandler::CreateWebMercatorModel method. The caller sets up the ImageryProvider from user input.
    DGNPLATFORM_EXPORT WebMercatorModel(CreateParams const& params);

    TileTree::RootPtr _CreateTileTree(Render::SystemP) override;
    void _OnSaveJsonProperties() override;
    void _OnLoadedJsonProperties() override;
    double GetGroundBias() const {return m_groundBias;}

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
    enum class MapType
        {
        StreetMap = 0,
        Satellite = 1,
        StreetsAndSatellite = 2,
        };

    BE_JSON_NAME(mapType)
    BE_PROP_NAME(MapBoxProvider)

private:
    Utf8String      m_baseUrl;
    MapType         m_mapType = MapType::StreetMap;

public:
    // constructor used prior to specifying from stored Json values.
    MapBoxImageryProvider () {};

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    virtual Utf8CP      _GetProviderName() const override { return prop_MapBoxProvider(); }

    // Gets the message to be displayed to credit provider(s). 
    virtual Utf8CP      _GetCreditMessage() const override;

    // Gets an URL for the image to be displayed to credit the provider.
    virtual Utf8CP      _GetCreditUrl() const override;

    // Gets the maximum zoom level alllowed (provider dependent)
    virtual int         _GetMaximumZoomLevel (bool forPrinting) override { return 19; }

    // Gets a root file name to use for the BeSQLite file into which we cache the tiles. Usually depends on provider and the map type returned
    virtual Utf8CP      _GetCacheFileName() const override;

    // Given the tile, constructs the URL needed to retrieve it. Different providers have different URL schemes.
    virtual Utf8String  _ConstructUrl (TileTree::QuadTree::Tile const& tile) const override;
    
    virtual void _FromJson(Json::Value const& value) override;

    virtual void _ToJson (Json::Value&) const override;
    };


//=======================================================================================
// Bing Imagery Provider
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct BingImageryProvider
    {
    BE_PROP_NAME(BingProvider)

    };

//=======================================================================================
// Google Imagery Provider (Here is the successor to NavTeq).
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct GoogleImageryProvider
    {
    BE_PROP_NAME(GoogleProvider)
    };

//=======================================================================================
// Here Imagery Provider (Here is the successor to NavTeq).
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct HereImageryProvider
    {
    BE_PROP_NAME(HereProvider)
    };


}; // end WebMercator namespace

//! @endGroup

END_BENTLEY_DGN_NAMESPACE
