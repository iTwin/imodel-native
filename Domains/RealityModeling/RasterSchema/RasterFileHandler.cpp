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
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileProperties::RasterFileProperties()
    {
    m_fileMonikerPtr = FileMoniker::Create("", "");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dFromJson (DRange2dR range, JsonValueCR inValue)
    {
    DPoint2dFromJson (range.low, inValue["low"]);
    DPoint2dFromJson (range.high, inValue["high"]);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
static void DRange2dToJson (JsonValueR outValue, DRange2dCR range)
    {
    DPoint2dToJson (outValue["low"], range.low);
    DPoint2dToJson (outValue["high"], range.high);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::ToJson(Json::Value& v) const
    {
    m_fileMonikerPtr->ToJson(v["fileMoniker"]);
    DRange2dToJson(v["bbox"], m_boundingBox);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::FromJson(Json::Value const& v)
    {
    m_fileMonikerPtr->FromJson(v["fileMoniker"]);
    DRange2dFromJson(m_boundingBox, v["bbox"]);
    }

//----------------------------------------------------------------------------------------
//------------------------------  RasterFileModelHandler  --------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
DgnModelId RasterFileModelHandler::CreateRasterFileModel(DgnDbR db, FileMonikerPtr fileMoniker)
    {
    DgnClassId classId(db.Schemas().GetECClassId(BENTLEY_RASTER_SCHEMA_NAME, RASTER_CLASSNAME_RasterFileModel));
    BeAssert(classId.IsValid());

    // Find resolved file name for the raster
    BeFileName basePath(db.GetDbFileName());
    Utf8String basePathUtf8(basePath);
    Utf8String resolvedName;
    fileMoniker->ResolveFileName(resolvedName, basePathUtf8);

    // Create model name (just use the file name without extension)
    BeFileName fileName(resolvedName);
    WString modelName ( fileName.GetFileNameWithoutExtension().c_str() );
    Utf8String utf8Name(modelName);

    // Set RasterFileProperties
    RasterFileProperties props;
    props.m_fileMonikerPtr = fileMoniker;

    // Open raster
    RasterFilePtr rasterFilePtr = RasterFile::Create(resolvedName);
    if (rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid. Return an invalid model id.
        return DgnModelId();
        }

    // Find raster range (in the DgnDb world)
    DRange2d rasterRange;
    if (REPROJECT_Success != GetRasterExtentInUors(rasterRange, *rasterFilePtr, db))
        {
        // Can't get raster extent. Return invalid model.
        return DgnModelId();
        }
    props.m_boundingBox = rasterRange;

    // Create model in DgnDb
    RasterFileModelPtr model = new RasterFileModel(DgnModel::CreateParams(db, classId, utf8Name.c_str()), props);
    model->Insert();
    return model->GetModelId();
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
    if(NULL == sourceGcsPtr.get() || NULL == pDgnGcs)
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
BentleyStatus RasterFileModel::_LoadQuadTree()
    {
    m_rasterTreeP = nullptr;

    // Resolve raster name
    BeFileName basePath(GetDgnDb().GetDbFileName());
    Utf8String basePathUtf8(basePath);
    Utf8String resolvedName;
    m_fileProperties.m_fileMonikerPtr->ResolveFileName(resolvedName, basePathUtf8);

    // Create RasterQuadTree
    RasterSourcePtr pSource = RasterFileSource::Create(resolvedName);
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



// POINTCLOUD_WIP_GR06 - Temporary location for FileMoniker (until we decide how to handle local file names)

//----------------------------------------------------------------------------------------
//------------------------------------  FileMoniker  -------------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
FileMoniker::FileMoniker (Utf8StringCR fullPath, Utf8StringCR basePath)
    {
    m_fullPath = fullPath;
    m_basePath = basePath;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
FileMonikerPtr FileMoniker::Create (Utf8StringCR fullPath, Utf8StringCR basePath)
    {
    return new FileMoniker(fullPath, basePath);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
BentleyStatus FileMoniker::ResolveFileName (Utf8StringR resolvedName, Utf8StringCR basePath) const
    {
    BeFileName fullPath(m_fullPath);
    BeFileName basePathWhenCreated(m_basePath);
    WString relativePath;

    // Find relative path according to the creation paths.
    // E.g. if fullPath == "d:\dir1\dir2\file.jpg" and basePathWhenCreated == "d:\dir1\"
    //      then relativePath will be equal to "dir2\file.jpg"
    BeFileName::FindRelativePath(relativePath, fullPath.c_str(), basePathWhenCreated.c_str());

    // Find full path relatively to current base path. Current base path is a directory and may contain a file name (probably the name of the dgndb).
    // E.g. if relativePath == "dir2\file.jpg" and currentBasePath == "d:\dir5\dir6\myDgnDb.dgndb"
    //      then relativePath will be equal to "d:\dir5\dir6\dir2\file.jpg"
    WString resolvedNameW;
    BeFileName currentBasePath(basePath);
    BentleyStatus status = BeFileName::ResolveRelativePath(resolvedNameW, relativePath.c_str(), currentBasePath.c_str());
    if (status == SUCCESS)
        {
        Utf8String resolvedNameUtf8(resolvedNameW);
        resolvedName = resolvedNameUtf8;
        }

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void FileMoniker::ToJson (JsonValueR outValue) const
    {
    outValue["fullPath"] = m_fullPath.c_str();
    outValue["basePath"] = m_basePath.c_str();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void FileMoniker::FromJson (JsonValueCR inValue)
    {
    m_fullPath = inValue["fullPath"].asString();
    m_basePath = inValue["basePath"].asString();
    }