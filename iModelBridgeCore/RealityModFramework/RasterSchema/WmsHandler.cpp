/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/WmsHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/WmsHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "WmsSource.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(WmsModelHandler)

//&&MM from json utils. need make these public
//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dFromJson (DPoint2dR point, JsonValueCR inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint2dToJson (JsonValueR outValue, DPoint2dCR point)
    {
    outValue[0] = point.x;
    outValue[1] = point.y;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
static void DRange2dFromJson (DRange2dR range, JsonValueCR inValue)
    {
    DPoint2dFromJson (range.low, inValue["low"]);
    DPoint2dFromJson (range.high, inValue["high"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
static void DRange2dToJson (JsonValueR outValue, DRange2dCR range)
    {
    DPoint2dToJson (outValue["low"], range.low);
    DPoint2dToJson (outValue["high"], range.high);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
 WmsProperties::WmsProperties()
    {
    //m_styles = "";      // default style
    m_format = "image/png";

    //m_vendorSpecific = "";
    m_transparent = false;
    }

//     Utf8String m_url;               //! Get map url. Up to but excluding the query char '?'
// 
//     // Raster window
//     DRange2d m_boundingBox;         //! Bounding box corners (minx,miny,maxx,maxy) in 'CoordinateSystem' unit.
//     uint32_t m_metaWidth;           //! Width of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this width.
//     uint32_t m_metaHeight;          //! Height of the window in pixels. The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distortion. Sub-Resolutions will be generated from this Height.
// 
// 
//     // Mandatory GetMap parameters
//     Utf8String m_version;           //! Wms server version
//     Utf8String m_layers;            //! Comma-separated list of one or more map layers.
//     Utf8String m_styles;            //! Comma-separated list of one rendering style per requested layer.
//     Utf8String m_csType;            //! Usually, 'SRS' for 1.1.1 and below. 'CRS' for 1.3 and above.
//     Utf8String m_csLabel;           //! Coordinate system label. ex: "EPSG:4326"
//     Utf8String m_format;            //! Output format of map default is 'image/png'
// 
//     // Optional parameters
//     Utf8String m_vendorSpecific;    //! [optional] Unparsed, vendor specific parameters that will be appended to the request. 
    

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsProperties::ToJson(Json::Value& v) const
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
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void WmsProperties::FromJson(Json::Value const& v)
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
    m_transparent = wmsValue["transp"].asBool();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
DgnPlatform::DgnModelId WmsModelHandler::CreateWmsModel(DgnDbR db, Utf8CP modelName, WmsProperties const& prop)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, RASTER_CLASSNAME_WmsModel));
    BeAssert(classId.IsValid());

    if(!prop.HasValidParameters())
        return DgnModelId();  // Can't create model, Return an invalid model id.

    WmsModelPtr modelP = new WmsModel(DgnModel::CreateParams(db, classId, modelName), prop);

    db.Models().Insert(*modelP);
    return modelP->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::WmsModel(CreateParams const& params) 
:T_Super (params)
    {
    // Make sure GCS is initialized. required to init csmap data.
    T_HOST.GetGeoCoordinationAdmin()._GetServices();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::WmsModel(CreateParams const& params, WmsProperties const& wmsMap) 
:T_Super (params),
 m_map(wmsMap)
    {
    // Make sure GCS is initialized. required to init csmap data.
    T_HOST.GetGeoCoordinationAdmin()._GetServices();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
WmsModel::~WmsModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
BentleyStatus WmsModel::_LoadQuadTree()
    {
    m_rasterTreeP = nullptr;

    RasterSourcePtr pSource = WmsSource::Create(m_map);
    if(pSource.IsValid())
        m_rasterTreeP = RasterQuadTree::Create(*pSource, GetDgnDb());

    //&&MM what about range or other stuff from the base?

    return m_rasterTreeP.IsValid() ? BSISUCCESS : BSIERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void WmsModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_map.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void WmsModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_map.FromJson(v);
    }

