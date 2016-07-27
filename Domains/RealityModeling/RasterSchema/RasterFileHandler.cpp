/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileProperties::RasterFileProperties()
    :m_fileId("")
    {
    m_transform.InitIdentity();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dFromJson (DRange2dR range, JsonValueCR inValue)
    {
    JsonUtils::DPoint2dFromJson (range.low, inValue["low"]);
    JsonUtils::DPoint2dFromJson (range.high, inValue["high"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     7/2016
//----------------------------------------------------------------------------------------
static void DMatrix4dFromJson (DMatrix4dR matrix, JsonValueCR inValue)
    {
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            matrix.coff[y][x] = inValue[y][x].asDouble();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     7/2016
//----------------------------------------------------------------------------------------
static void DMatrix4dToJson (JsonValueR outValue, DMatrix4dCR matrix)
    {
    for (int y = 0; y < 4; y++)
        for (int x = 0; x < 4; x++)
            outValue[y][x] = matrix.coff[y][x];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dToJson (JsonValueR outValue, DRange2dCR range)
    {
    JsonUtils::DPoint2dToJson (outValue["low"], range.low);
    JsonUtils::DPoint2dToJson (outValue["high"], range.high);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::ToJson(Json::Value& v) const
    {
    v["fileId"] = m_fileId.c_str();
    DRange2dToJson(v["bbox"], m_boundingBox);
    DMatrix4dToJson(v["transform"], m_transform);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::FromJson(Json::Value const& v)
    {
    m_fileId = v["fileId"].asString();
    DRange2dFromJson(m_boundingBox, v["bbox"]);
    DMatrix4dFromJson(m_transform, v["transform"]);
    }

//----------------------------------------------------------------------------------------
//------------------------------  RasterFileModelHandler  --------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModelPtr RasterFileModelHandler::CreateRasterFileModel(RasterFileModel::CreateParams const& params)
    {
    // unused - DgnClassId classId(params.m_dgndb.Schemas().GetECClassId(RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel));
    // Find resolved file name for the raster
    BeFileName fileName;
    BentleyStatus status = T_HOST.GetRasterAttachmentAdmin()._ResolveFileName(fileName, params.m_fileId, params.m_dgndb);
    if (status != SUCCESS)
        {
        return nullptr;
        }
    Utf8String resolvedName(fileName);
    Utf8String modelName(fileName.GetFileNameWithoutExtension().c_str());

    // Open raster
    RasterFilePtr rasterFilePtr = RasterFile::Create(resolvedName);
    if (rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid.
        return nullptr;
        }

    // Find raster range (in the DgnDb world)
    DRange2d rasterRange;
    if (REPROJECT_Success != GetRasterExtentInUors(rasterRange, *rasterFilePtr, params.m_dgndb))
        {
        // Can't get raster extent.
        return nullptr;
        }

    RasterFileProperties props;
    props.m_fileId = params.m_fileId;
    props.m_boundingBox = rasterRange;

    if (params.m_transformP != nullptr)
        props.m_transform = *params.m_transformP;
    else
        {
        //&&ep - is this ok ?
        props.m_transform = rasterFilePtr->GetGeoTransform();
        }


    // Create model in DgnDb
    RasterFileModelPtr model = new RasterFileModel(params, props);

    return model;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
static ReprojectStatus s_FilterGeocoordWarning(ReprojectStatus status)
    {
    if(REPROJECT_CSMAPERR_OutOfUsefulRange == status)   // This a warning
        return REPROJECT_Success;

    return status;   
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     7/2015
//----------------------------------------------------------------------------------------
ReprojectStatus RasterFileModelHandler::GetRasterExtentInUors(DRange2d &range, RasterFileCR rasterFile, DgnDbCR db) 
    {
    DPoint3d srcCornersCartesian[4];
    rasterFile.GetCorners(srcCornersCartesian);

    // Use raster GCS as source
    GeoCoordinates::BaseGCSPtr sourceGcsPtr = rasterFile.GetBaseGcs();
    DgnGCSP pDgnGcs = db.Units().GetDgnGCS();

    ReprojectStatus status = REPROJECT_Success;
    DPoint3d dgnCornersUors[4];
    if(NULL == sourceGcsPtr.get() || NULL == pDgnGcs || !sourceGcsPtr->IsValid() || !pDgnGcs->IsValid())
        {
        // Assume raster to be coincident.
        memcpy(dgnCornersUors, srcCornersCartesian, sizeof(DPoint3d)*4);
        }
    else
        {
        GeoPoint srcCornersLatLong[4];
        for(size_t i=0; i < 4; ++i)
            {
            if(REPROJECT_Success != (status = s_FilterGeocoordWarning(sourceGcsPtr->LatLongFromCartesian(srcCornersLatLong[i], srcCornersCartesian[i]))))
                {
                BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
                return status;
                }
            }

        // Source latlong to DgnDb latlong.
        GeoPoint dgnCornersLatLong[4];
        for(size_t i=0; i < 4; ++i)
            {
            if(REPROJECT_Success != (status = s_FilterGeocoordWarning(sourceGcsPtr->LatLongFromLatLong(dgnCornersLatLong[i], srcCornersLatLong[i], *pDgnGcs))))
                return status;
            }

        //Finally to UOR
        for(uint32_t i=0; i < 4; ++i)
            {
            if(REPROJECT_Success != (status = s_FilterGeocoordWarning(pDgnGcs->UorsFromLatLong(dgnCornersUors[i], dgnCornersLatLong[i]))))
                return status;
            }
        }

    // Extract extent
    double minX = dgnCornersUors[0].x;
    double minY = dgnCornersUors[0].y;
    double maxX = dgnCornersUors[0].x;
    double maxY = dgnCornersUors[0].y;
    for(uint32_t i=1; i < 4; ++i)
        {
        if (dgnCornersUors[i].x < minX)
            minX = dgnCornersUors[i].x;
        if (dgnCornersUors[i].y < minY)
            minY = dgnCornersUors[i].y;
        if (dgnCornersUors[i].x > maxX)
            maxX = dgnCornersUors[i].x;
        if (dgnCornersUors[i].y > maxY)
            maxY = dgnCornersUors[i].y;
        }
    range.low.x = minX;
    range.low.y = minY;
    range.high.x = maxX;
    range.high.y = maxY;

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params) 
:T_Super (params)
    {

    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params, RasterFileProperties const& properties) 
:T_Super (params),
 m_fileProperties(properties)
    {

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
BentleyStatus RasterFileModel::_LoadQuadTree() const
    {
    m_rasterTreeP = nullptr;
   
    // Resolve raster name
    BeFileName fileName;
    BentleyStatus status = T_HOST.GetRasterAttachmentAdmin()._ResolveFileName(fileName, m_fileProperties.m_fileId, GetDgnDb());
    if (status != SUCCESS)
        {
        return ERROR;
        }
    Utf8String resolvedName(fileName);

    // Create RasterQuadTree
    RasterSourcePtr pSource = RasterFileSource::Create(resolvedName);
    if(pSource.IsValid())
        m_rasterTreeP = RasterQuadTree::Create(*pSource, GetDgnDb());

    return m_rasterTreeP.IsValid() ? SUCCESS : ERROR;
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
void RasterFileModel::_WriteJsonProperties(Json::Value& v) const
    {
    T_Super::_WriteJsonProperties(v);
    m_fileProperties.ToJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_ReadJsonProperties(Json::Value const& v)
    {
    T_Super::_ReadJsonProperties(v);
    m_fileProperties.FromJson(v);
    }
