/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterInternal.h>
#include <Raster/WmsHandler.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTER

HANDLER_DEFINE_MEMBERS(WmsModelHandler)

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
static void DRange2dFromJson (DRange2dR range, JsonValueCR inValue)
    {
    JsonUtils::DPoint2dFromJson (range.low, inValue["low"]);
    JsonUtils::DPoint2dFromJson (range.high, inValue["high"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
static void DRange2dToJson (JsonValueR outValue, DRange2dCR range)
    {
    JsonUtils::DPoint2dToJson (outValue["low"], range.low);
    JsonUtils::DPoint2dToJson (outValue["high"], range.high);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsMap::WmsMap()
    {
    m_boundingBox.Init(); // null range
    m_metaWidth = 0;
    m_metaHeight = 0;
    m_transparent = false;
    m_axisOrder = AxisOrder::Default;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
 WmsMap::WmsMap(Utf8CP url, DRange2dCR bbox, Utf8CP version, Utf8CP layers, Utf8CP csLabel)
 :m_url(url),
  m_boundingBox(bbox),
  //m_metaWidth(),
  //m_metaHeight(),
  m_version(version),
  m_layers(layers),
  m_styles(""),         // Default style.
  //m_csType(""), 
  m_csLabel(csLabel),
  m_format("image/png"),    // The standards says that all servers should support png.
  m_vendorSpecific(""),
  m_transparent(false),
  m_axisOrder(AxisOrder::Default)
    {
    BeAssert(m_url.at(m_url.size()-1) != '?');

    // 1.3.x use CRS.
    if(m_version.size() >= sizeof("1.3") && memcmp(m_version.c_str(), "1.3", sizeof("1.3")-1) == 0)
        {
        m_csType = "CRS"; 
        }
    else     // 1.0.x and 1.1.x use SRS.
        {
        m_csType = "SRS";
        }

    SetMetaSizeByResolutionCount(10);
    }
    
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsMap::SetMetaSizeByResolutionCount(uint32_t count)
    {
    // Limit to what a signed int can hold.
    SetMetaSizeByLargestBoundingBoxSide((uint32_t)MIN((256 << (uint64_t)count), INT32_MAX));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsMap::SetMetaSizeByLargestBoundingBoxSide(uint32_t pixelCount)
    {
    // Limit to what a signed int can hold.
    pixelCount = MIN(pixelCount, INT32_MAX);

    double xLength = m_boundingBox.high.x - m_boundingBox.low.x;
    double yLength = m_boundingBox.high.y - m_boundingBox.low.y;

    // Keep the same cartesian to pixel ratio
    if(xLength > yLength)
        {
        m_metaWidth = pixelCount;
        m_metaHeight = (uint32_t)((yLength/xLength) * pixelCount);
        }
    else
        {
        m_metaHeight = pixelCount;
        m_metaWidth = (uint32_t)((xLength/yLength) * pixelCount);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
bool WmsMap::HasValidParameters() const 
    {
    if(m_url.empty() || m_boundingBox.IsNull() || 0 == m_metaWidth || 0 == m_metaHeight ||
        m_version.empty() || m_layers.empty() || m_csType.empty() || m_csLabel.empty() || m_format.empty())
        return false;
    
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsMap::ToJson(Json::Value& v) const
    {
    Json::Value& wmsValue = v["wmsMap"];

    wmsValue["url"] = m_url.c_str();

    DRange2dToJson(wmsValue["bbox"], m_boundingBox);
    wmsValue["metaWidth"] = m_metaWidth;
    wmsValue["metaHeight"] = m_metaHeight;

    wmsValue["ver"] = m_version.c_str();
    wmsValue["layer"] = m_layers.c_str();
    wmsValue["style"] = m_styles.c_str();
    wmsValue["csType"] = m_csType.c_str();
    wmsValue["csLabel"] = m_csLabel.c_str();
    wmsValue["format"] = m_format.c_str();

    wmsValue["vendorSpec"] = m_vendorSpecific.c_str();
    wmsValue["transp"] = m_transparent;
    wmsValue["axisOrder"] = (uint32_t)m_axisOrder;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsMap::FromJson(Json::Value const& v)
    {
    Json::Value const& wmsValue = v["wmsMap"];

    m_url = wmsValue["url"].asString();

    DRange2dFromJson(m_boundingBox, wmsValue["bbox"]);
    m_metaWidth = wmsValue["metaWidth"].asUInt();
    m_metaHeight = wmsValue["metaHeight"].asUInt();

    m_version = wmsValue["ver"].asString();
    m_layers = wmsValue["layer"].asString();
    m_styles = wmsValue["style"].asString();
    m_csType = wmsValue["csType"].asString();
    m_csLabel = wmsValue["csLabel"].asString();
    m_format = wmsValue["format"].asString();

    m_vendorSpecific = wmsValue["vendorSpec"].asString();
    m_transparent = wmsValue.get("transp", Json::Value(false)).asBool();
    m_axisOrder = (AxisOrder)wmsValue.get("axisOrder", Json::Value((uint32_t)AxisOrder::Default)).asUInt();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModelPtr WmsModelHandler::CreateWmsModel(DgnDbR db, Dgn::RepositoryLinkCR link, WmsMap const& mapInfo)
    {
    if (!link.GetElementId().IsValid())        // link must be persisted.
        return nullptr;

    DgnClassId classId(db.Schemas().GetClassId(RASTER_SCHEMA_NAME, RASTER_CLASSNAME_WmsModel));
    BeAssert(classId.IsValid());

    if(!mapInfo.HasValidParameters())
        return nullptr;  // Can't create model.

    return new WmsModel(DgnModel::CreateParams(db, classId, link.GetElementId()), mapInfo);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::WmsModel(CreateParams const& params) 
:T_Super (params)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::WmsModel(CreateParams const& params, WmsMap const& wmsMap) 
:T_Super (params),
 m_map(wmsMap)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::~WmsModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void WmsModel::_OnSaveJsonProperties() 
    {
    T_Super::_OnSaveJsonProperties();
    Json::Value val;
    m_map.ToJson(val);
    SetJsonProperties(json_wms(), val);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void WmsModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();
    m_map.FromJson(GetJsonProperties(json_wms()));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
WmsMap const& WmsModel::GetMap() const
    {
    return m_map;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
Http::HttpStatus WmsModel::GetLastHttpError() const
    {
    return Http::HttpStatus::None;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
Http::HttpStatus WmsModel::Authenticate(Http::Credentials const& credentials, Http::Credentials const& proxyCredentials)
    {
    BeAssert(false);
    return Http::HttpStatus::None;
    }



