/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDbTables.h>
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(WebMercatorModel)

enum class MapType : int {None=0, Street=1, Aerial=2, Hybrid=3};

//=======================================================================================
// Interface between QuadTree implementation and the providers of the imagery.
// @bsiclass                                                    Barry.Bentley   03/17
//=======================================================================================
struct ImageryProvider : RefCountedBase
{
    virtual ~ImageryProvider () {}

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    virtual Utf8String _GetProviderName() const = 0;

    // Gets the Json that is saved to the model and used to reconstruct the ImageryProvider in subsequent sessions.
    virtual void _ToJson(Json::Value&) const = 0;

    // Reconstructs the ImageryProvider parameters from the Json that is saved to the model.
    virtual void _FromJson(Json::Value const& value) = 0;
    virtual MapType _GetMapType () const = 0;
};

DEFINE_REF_COUNTED_PTR(ImageryProvider)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ImageryProvider)

//=======================================================================================
//! Obtains and displays multi-resolution tiled raster organized according to the WebMercator tiling system.
// @bsiclass                                                    Sam.Wilson      10/2014
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE WebMercatorModel : SpatialModel
{
    friend struct FetchTemplateUrlProgressiveTask;

    DGNMODEL_DECLARE_MEMBERS("WebMercatorModel", SpatialModel);

protected:
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
private:
    Utf8String m_baseUrl;
    MapType m_mapType = MapType::Street;

    // constructor used prior to specifying from stored Json values.
    MapBoxImageryProvider() {}
public:
    BE_PROP_NAME(MapBoxProvider)

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_MapBoxProvider();}
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
private:
    MapType m_mapType = MapType::Street;

    static BingImageryProviderPtr   s_streetMapProvider;
    static BingImageryProviderPtr   s_aerialMapProvider;
    static BingImageryProviderPtr   s_hybridMapProvider;

    // constructor used prior to specifying from stored Json values.
    BingImageryProvider () { }

    virtual ~BingImageryProvider() { }

public:
    BE_PROP_NAME(BingProvider)

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_BingProvider();}
    void _FromJson(Json::Value const& value) override;
    void _ToJson(Json::Value&) const override;
    MapType _GetMapType () const override {return m_mapType; }

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
    MapType m_mapType = MapType::Street;

public:
    BE_PROP_NAME(HereProvider)

    // constructor used prior to specifying from stored Json values.
    HereImageryProvider();

    // returns the ProviderName. Saved to the model to select the right when the ImageryProvider is instantiated. Not translated.
    Utf8String _GetProviderName() const override {return prop_HereProvider();}
    void _FromJson(Json::Value const& value) override;
    void _ToJson(Json::Value&) const override;
    MapType _GetMapType () const override {return m_mapType; }

    static HereImageryProvider* Create (Json::Value const& providerDataValue);
    };

}; // end WebMercator namespace

END_BENTLEY_DGN_NAMESPACE
