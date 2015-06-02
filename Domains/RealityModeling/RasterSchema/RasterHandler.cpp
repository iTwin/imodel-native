/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "WmsSource.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId RasterModelHandler::CreateRasterModel(DgnDbR db, BeFileName fileName)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, "RasterModel"));
    BeAssert(classId.IsValid());

    WString dev;
    WString dir;
    WString name;
    WString ext;
    fileName.ParseName(&dev, &dir, &name, &ext);

    // Just keep model name (no path or extension) and convert it to Utf8CP
    Utf8String utf8Name;
    BeStringUtilities::WCharToUtf8 (utf8Name, name.c_str());
    Utf8CP modelName = utf8Name.c_str();

    RasterModelPtr model = new RasterModel(DgnModel::CreateParams(db, classId, modelName));
    if (model->SetProperties(fileName) != SUCCESS)
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();

    db.Models().Insert(*model);
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::RasterModel(CreateParams const& params) : T_Super (params)
    {
    // Make sure GCS is initialized
    T_HOST.GetGeoCoordinationAdmin()._GetServices();

    //http://basemap.nationalmap.gov/arcgis/services/USGSImageryTopo/MapServer/WMSServer?request=GetCapabilities&service=WMS

    //&&MM everything is hardcoded for now.
    Utf8String serverUrl = "http://basemap.nationalmap.gov/arcgis/services/USGSImageryTopo/MapServer/WmsServer";
    Utf8String version = "1.1.1";
    Utf8String layers = "0";
    Utf8String styles = "";
    Utf8String csType = "SRS";
    Utf8String csLabel = "EPSG:4269";
    Utf8String format = "image/png";
    Utf8String transparent = "TRUE";
           
    DRange2d bbox;
    bbox.low.x = -128.1;
    bbox.low.y = 24.7;
    bbox.high.x = -58.1;
    bbox.high.y = 49.8;   

    // Keep the same cartesian to pixel ratio
    double xLength = bbox.high.x - bbox.low.x;
    double yLength = bbox.high.y - bbox.low.y;
                        
    uint32_t metaWidth = 2048*256;  // arbitrary size. &&MM let the user decide? compute something automatically using bbox? server res? or a default?
    uint32_t metaHeight = (uint32_t)((yLength/xLength) * metaWidth);
                        
    m_rasterTree = RasterQuadTree::Create(*WmsSource::Create(serverUrl.c_str(), version.c_str(), layers.c_str(), styles.c_str(), csType.c_str(), csLabel.c_str(), format.c_str(), bbox, metaWidth, metaHeight), GetDgnDb());
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_AddGraphicsToScene (ViewContextR context)
    {
    m_rasterTree->Draw(context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterModel::_QueryModelRange() const
    {
    return m_properties.m_range;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
BentleyStatus RasterModel::SetProperties (BeFileName fileName)
    {
    WString fileNameWithPath;
    BeFileName::BuildName (fileNameWithPath, fileName.GetDevice().c_str(), fileName.GetDirectoryWithoutDevice().c_str(), fileName.GetFileNameWithoutExtension().c_str(), fileName.GetExtension().c_str());
    BeStringUtilities::WCharToUtf8 (m_properties.m_URL, fileNameWithPath.c_str());

//    m_properties.m_range = GetSceneRange();

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    }

