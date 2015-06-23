/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterFileHandler.h>
#include "RasterSource.h"
#include "RasterQuadTree.h"
#include "RasterFileSource.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterFileModelHandler)

//----------------------------------------------------------------------------------------
//-------------------------------  RasterFileProperties  ---------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::ToJson(Json::Value& v) const
    {
    v["url"] = m_url.c_str();

    //&&EP todo DRange2dToJson(v["bbox"], m_boundingBox);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::FromJson(Json::Value const& v)
    {
    m_url = v["url"].asString();

    //&&EP todo DRange2dFromJson(m_boundingBox, v["bbox"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId RasterFileModelHandler::CreateRasterFileModel(DgnDbR db, BeFileName fileName)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel));
    BeAssert(classId.IsValid());

    WString dev;
    WString dir;
    WString name;
    WString ext;
    fileName.ParseName(&dev, &dir, &name, &ext);

    // Just keep model name (no path or extension) and convert it to Utf8CP
    Utf8String utf8Name(name);
    Utf8CP modelName = utf8Name.c_str();

    // Set RasterFileProperties
    RasterFileProperties props;
    WString fileNameWithPath;
    BeFileName::BuildName (fileNameWithPath, fileName.GetDevice().c_str(), fileName.GetDirectoryWithoutDevice().c_str(), fileName.GetFileNameWithoutExtension().c_str(), fileName.GetExtension().c_str());
    BeStringUtilities::WCharToUtf8 (props.m_url, fileNameWithPath.c_str());

    //&&ep - Open raster; set other properties
    RasterFilePtr rasterFilePtr = RasterFile::Create(fileName.c_str());
    if (rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();
        }

    Point2d sizePixels;
    rasterFilePtr->GetSize(&sizePixels);

    RasterFileModelPtr model = new RasterFileModel(DgnModel::CreateParams(db, classId, modelName), props);

    model->Insert();
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params) 
:T_Super (params)
    {
    //&&ep need this here ? or maybe in Register domain instead.
    // Make sure GCS is initialized
    T_HOST.GetGeoCoordinationAdmin()._GetServices();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params, RasterFileProperties const& properties) 
:T_Super (params),
 m_fileProperties(properties)
    {
    // &&ep need this here ? or maybe in Register domain instead.
    // Make sure GCS is initialized
    T_HOST.GetGeoCoordinationAdmin()._GetServices();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::~RasterFileModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
BentleyStatus RasterFileModel::_LoadQuadTree()
    {
    m_rasterTreeP = nullptr;

    RasterSourcePtr pSource = RasterFileSource::Create(m_fileProperties);
    if(pSource.IsValid())
        m_rasterTreeP = RasterQuadTree::Create(*pSource, GetDgnDb());

    return m_rasterTreeP.IsValid() ? BSISUCCESS : BSIERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterFileModel::_QueryModelRange() const
    {
    return AxisAlignedBox3d(m_fileProperties.m_boundingBox);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    m_fileProperties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    m_fileProperties.FromJson(v);
    }

