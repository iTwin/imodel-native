/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::ToJson(BeJsValue value) const
    {
    value.SetEmptyObject();
    value[json_providerName()] = m_provider->_GetProviderName();
    value[json_groundBias()]   = m_groundBias;
    value[json_transparency()] = m_transparency;
    m_provider->_ToJson(value[json_providerData()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::FromJson(BeJsConst value)
    {
    m_groundBias = value[json_groundBias()].asDouble(-1.0);
    m_transparency = value[json_transparency()].asDouble(0.0);
    LIMIT_RANGE(0.0, .95, m_transparency);

    Utf8String providerName = value[json_providerName()].asString();
    auto providerDataValue = value[json_providerData()];

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnSaveJsonProperties()
    {
    BeJsDocument value;
    ToJson(value);
    SetJsonProperties(json_webMercatorModel(), value);
    T_Super::_OnSaveJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void WebMercatorModel::_OnLoadedJsonProperties()
    {
    auto value = GetJsonProperties(json_webMercatorModel());
    FromJson(value);
    T_Super::_OnLoadedJsonProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MapBoxImageryProvider*  MapBoxImageryProvider::Create (BeJsConst providerDataValue)
    {
    MapBoxImageryProvider*    imageryProvider = new MapBoxImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_FromJson(BeJsConst value)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MapBoxImageryProvider::_ToJson(BeJsValue value) const
    {
    // the only thing currently stored in the MapBoxImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

BingImageryProviderPtr  BingImageryProvider::s_streetMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_aerialMapProvider;
BingImageryProviderPtr  BingImageryProvider::s_hybridMapProvider;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BingImageryProvider*  BingImageryProvider::Create (BeJsConst providerDataValue)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_FromJson(BeJsConst value)
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    BingImageryProvider::_ToJson(BeJsValue value) const
    {
    // the only thing currently stored in the BingImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider*  HereImageryProvider::Create (BeJsConst providerDataValue)
    {
    HereImageryProvider*    imageryProvider = new HereImageryProvider();
    imageryProvider->_FromJson (providerDataValue);

    // if the mapType is None, return null.
    if (MapType::None == imageryProvider->m_mapType)
        return nullptr;

    return imageryProvider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HereImageryProvider::HereImageryProvider()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_FromJson(BeJsConst value)
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    m_mapType = (MapType) value[WebMercatorModel::json_mapType()].asInt((int)MapType::Street);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void    HereImageryProvider::_ToJson(BeJsValue value) const
    {
    // the only thing currently stored in the HereImageryProvider Json is the MapType.
    value[WebMercatorModel::json_mapType()] = (int)m_mapType;
    }
