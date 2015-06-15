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

//&&ep d static HFCPtr<HRFRasterFile> GetRasterFile(WCharCP inFilename);

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

/* &&ep - useless now
    if (model->SetProperties(fileName) != SUCCESS)
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();
*/

    db.Models().Insert(*model);
    return model->GetModelId();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params) 
:T_Super (params)
    {
//&&ep - not needed ?
    m_rasterFilePtr = nullptr;

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
//&&ep need this here ? or maybe in Register domain instead.
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

/* &&ep d
    RasterSourcePtr pSource = WmsSource::Create(m_map);
    if(pSource.IsValid())
        m_rasterTreeP = RasterQuadTree::Create(*pSource, GetDgnDb());

    //&&MM what about range or other stuff from the base?
*/
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
BentleyStatus RasterFileModel::SetProperties (BeFileNameCR fileName)
    {
    WString fileNameWithPath;
    BeFileName::BuildName (fileNameWithPath, fileName.GetDevice().c_str(), fileName.GetDirectoryWithoutDevice().c_str(), fileName.GetFileNameWithoutExtension().c_str(), fileName.GetExtension().c_str());
//&&ep o    BeStringUtilities::WCharToUtf8 (m_fileProperties.m_URL, fileNameWithPath.c_str());
    BeStringUtilities::WCharToUtf8 (m_fileProperties.m_url, fileNameWithPath.c_str());

//&&ep o    HFCPtr<HRFRasterFile> rasterFile = GetRasterFile(fileNameWithPath.c_str());
//&&ep d    m_rasterFilePtr = RasterFile::Create(fileNameWithPath.c_str());

//&&ep need here ?
    if (GetRasterFilePtr()->GetHRFRasterFileP() == nullptr)
        return ERROR;

//    m_fileProperties.m_range = GetSceneRange();

    return SUCCESS;
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFilePtr RasterFileModel::GetRasterFilePtr()
    {
    if (m_rasterFilePtr == nullptr)
        {
        // Open raster file
//&&ep o        Utf8CP urlUtf8 = m_fileProperties.m_URL.c_str();
        Utf8CP urlUtf8 = m_fileProperties.m_url.c_str();
        WString fileName(urlUtf8,BentleyCharEncoding::Utf8);
        m_rasterFilePtr = RasterFile::Create(fileName.c_str());
        }

    return m_rasterFilePtr;
    }
