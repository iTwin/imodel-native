/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnGeoCoord.h>

using namespace WebMercator;

namespace WebMercatorStrings
{
// top level identifier of WebMercator subfolder.
BE_JSON_NAME(webMercatorModel)

// property names common to all providers
};

using namespace WebMercatorStrings;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::ToJson(Json::Value& value) const
    {
    value[json_providerName()] = m_provider->_GetProviderName();
    value[json_groundBias()]   = m_groundBias;
    value[json_transparency()] = m_transparency;

    m_provider->_ToJson(value[json_providerData()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::FromJson(Json::Value const& value)
    {
    m_groundBias = value[json_groundBias()].asDouble(-1.0);
    m_transparency = value[json_transparency()].asDouble(0.0);
    LIMIT_RANGE(0.0, .95, m_transparency);

    Utf8String providerName = value[json_providerName()].asString();
    Json::Value const& providerDataValue = value[json_providerData()];

    if (0 == providerName.CompareToI(WebMercator::MapBoxImageryProvider::prop_MapBoxProvider()))
        {
        m_provider = MapBoxImageryProvider::Create(providerDataValue);
        }
    else if (0 == providerName.CompareToI(WebMercator::BingImageryProvider::prop_BingProvider()))
        {
        m_provider = BingImageryProvider::Create(providerDataValue);
        }
    else if (0 == providerName.CompareToI(WebMercator::HereImageryProvider::prop_HereProvider()))
        {
        m_provider = HereImageryProvider::Create(providerDataValue);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnSaveJsonProperties()
    {
    Json::Value value;
    ToJson(value);
    SetJsonProperties(json_webMercatorModel(), value);
    T_Super::_OnSaveJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnLoadedJsonProperties()
    {
    ECN::AdHocJsonValueCR value = GetJsonProperties(json_webMercatorModel());

    FromJson(value);
    T_Super::_OnLoadedJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
WebMercatorModel::WebMercatorModel(CreateParams const& params) : T_Super(params)
    {
    // if the jsonParameters aren't filled in, that's because this is creating an existing the model from the DgnDb.
    // We will get the json parameters later when _OnLoadedJsonProperties() is called.
    if (params.m_jsonParameters.isNull())
        return;

    // if not null, this is a new model creation. Get the parameters from those passed in to the constructor of CreateParams.
    FromJson(params.m_jsonParameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
MapBoxImageryProvider*  MapBoxImageryProvider::Create (Json::Value const& providerDataValue)
    {
    MapBoxImageryProvider*    imageryProvider = new MapBoxImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the MapBoxImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);

    switch (m_mapType)
        {
        case MapType::Street:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.streets/";
            break;

        case MapType::Aerial:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.satellite/";
            break;

        case MapType::Hybrid:
            m_baseUrl = "http://api.mapbox.com/v4/mapbox.streets-satellite/";
            break;

        default:
            BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the MapBoxImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

BingImageryProviderPtr  BingImageryProvider::s_streetMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_aerialMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_hybridMapProvider;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
BingImageryProvider*  BingImageryProvider::Create (Json::Value const& providerDataValue)
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    MapType mapType = (MapType) providerDataValue[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);

    // if the background tupe is None, return nullptr.
    if (MapType::None == mapType)
        return nullptr;

    // we keep one imagery provider for each MapType of map. That way we don't have to query
    // again for the URL to ask, and Bing's accounting might be based off those queries, so we use
    // as few as possible.

    BingImageryProviderPtr* providerForTypeP = nullptr;
    if (MapType::Street == mapType)
        providerForTypeP = &s_streetMapProvider;
    else if (MapType::Aerial == mapType)
        providerForTypeP = &s_aerialMapProvider;
    else if (MapType::Hybrid == mapType)
        providerForTypeP = &s_hybridMapProvider;

    if (nullptr == providerForTypeP)
        {
        BeAssert (false);
        providerForTypeP = &s_streetMapProvider;
        }

    if ((*providerForTypeP).IsNull())
        {
        (*providerForTypeP) = new BingImageryProvider();
        (*providerForTypeP)->_FromJson (providerDataValue);
        }

    return (*providerForTypeP).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider*  HereImageryProvider::Create (Json::Value const& providerDataValue)
    {
    HereImageryProvider*    imageryProvider = new HereImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider::HereImageryProvider()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_FromJson(Json::Value const& value)
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_ToJson(Json::Value& value) const
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }
