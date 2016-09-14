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
#include "RasterTileTree.h"
#include "RasterFileSource.h"

HANDLER_DEFINE_MEMBERS(RasterFileModelHandler)

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_RASTERSCHEMA


//----------------------------------------------------------------------------------------
//-------------------------------  RasterFileProperties  ---------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
RasterFileProperties::RasterFileProperties()
    :m_fileId("")
    {
    m_sourceToWorld.InitIdentity();
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
void RasterFileProperties::ToJson(Json::Value& v) const
    {
    v["fileId"] = m_fileId.c_str();
    DMatrix4dToJson(v["srcToBim"], m_sourceToWorld);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     6/2015
//----------------------------------------------------------------------------------------
void RasterFileProperties::FromJson(Json::Value const& v)
    {
    m_fileId = v["fileId"].asString();
    DMatrix4dFromJson(m_sourceToWorld, v["srcToBim"]);
    }

//----------------------------------------------------------------------------------------
//------------------------------  RasterFileModelHandler  --------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
static ReprojectStatus s_FilterGeocoordWarning(ReprojectStatus status)
    {
    if ((REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))   // These are warnings
        return REPROJECT_Success;

    return status;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
StatusInt RasterFileModelHandler::ComputeGeoLocationFromFile(DMatrix4dR sourceToWorld, RasterFileR raster, DgnDbR dgndb)
    {
    GeoCoordinates::BaseGCSPtr pSourceGcs = raster.GetBaseGcs();

    DgnGCSP pDgnGcs = dgndb.Units().GetDgnGCS();

    if (pSourceGcs.IsNull() || nullptr == pDgnGcs || !pSourceGcs->IsValid() || !pDgnGcs->IsValid() ||
        pSourceGcs->IsEquivalent(*pDgnGcs))
        {
        // Assume raster to be coincident.
        sourceToWorld = raster.GetGeoTransform();   // always in meters.
        return SUCCESS;
        }

    DPoint3d physicalCorners[4];
    physicalCorners[0].x = physicalCorners[2].x = 0.0;
    physicalCorners[1].x = physicalCorners[3].x = raster.GetWidth();
    physicalCorners[0].y = physicalCorners[1].y = 0.0;
    physicalCorners[2].y = physicalCorners[3].y = raster.GetHeight();
    physicalCorners[0].z = physicalCorners[1].z = physicalCorners[2].z = physicalCorners[3].z = 0.0;

    double seed[] = {0.10, 0.5, 0.90};
    //double seed[] = {0.0, 1.0}; // 4 corners only.
    size_t seedCount = sizeof(seed) / sizeof(seed[0]);
    std::vector<DPoint3d> tiePoints;

    DMatrix4d sourceToMeter = raster.GetGeoTransform();     // always in meters.

    for (size_t y = 0; y < seedCount; ++y)
        {
        for (size_t x = 0; x < seedCount; ++x)
            {
            ReprojectStatus status = REPROJECT_Success;

            DPoint3d pointPixel = DPoint3d::FromInterpolateBilinear(physicalCorners[0], physicalCorners[1], physicalCorners[2], physicalCorners[3], seed[x], seed[y]);

            DPoint3d srcCartesian;
            sourceToMeter.MultiplyAndRenormalize(srcCartesian, pointPixel);

            // Transform to GCS native units.
            srcCartesian.Scale(pSourceGcs->UnitsFromMeters());

            GeoPoint srcGeo;
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromCartesian(srcGeo, srcCartesian))))
                {
                BeAssert(!"A source should always be able to represent itself in its GCS."); // That operation cannot fail or can it?
                return ERROR;
                }

            // Source latlong to BIM latlong.
            GeoPoint bimGeo;
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pSourceGcs->LatLongFromLatLong(bimGeo, srcGeo, *pDgnGcs))))
                return ERROR;
            
            // Finally to UOR/BIM
            DPoint3d bimPoint;
            if (REPROJECT_Success != (status = s_FilterGeocoordWarning(pDgnGcs->UorsFromLatLong(bimPoint, bimGeo))))
                return ERROR;
            
            tiePoints.push_back(pointPixel);   // uncorrected
            tiePoints.push_back(bimPoint);     // corrected
            }
        }

    if (0 != ImagePP::HGF2DDCTransfoModel::GetAffineTransfoMatrixFromScaleAndTiePts(sourceToWorld.coff, (uint16_t) tiePoints.size() * 3, (double const*) tiePoints.data()))
        return ERROR;

    DMatrix4d worldToSource;
    worldToSource.QrInverseOf(sourceToWorld);

    bool isAffinePreciseEnough = true;
    for (size_t i = 0; i < tiePoints.size(); i += 2)
        {
        DPoint3d sourcePixel;
        worldToSource.MultiplyAndRenormalize(sourcePixel, tiePoints[i + 1]);
        if (fabs(sourcePixel.x - tiePoints[i].x) > 0.4999999 ||
            fabs(sourcePixel.y - tiePoints[i].y) > 0.4999999)
            {
            // Affine transform have an error greater than half a pixel use projective.
            isAffinePreciseEnough = false;
            break;
            }
        }

    if (isAffinePreciseEnough)
        return SUCCESS;

    // Use projective when affine is not precise enough.
    if (0 != ImagePP::HGF2DDCTransfoModel::GetProjectiveTransfoMatrixFromScaleAndTiePts(sourceToWorld.coff, (uint16_t) tiePoints.size() * 3, (double const*) tiePoints.data()))
        return ERROR;
    
    return SUCCESS;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModelPtr RasterFileModelHandler::CreateRasterFileModel(RasterFileModel::CreateParams const& params)
    {
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
    //&&MM &&ep We must restructure that Create so rasterFilePtr is not lost but somehow reuse as input parameter to RasterFileModel.  
    // Otherwise the raster file is opened twice and this might be slow for network connection.
    RasterFilePtr rasterFilePtr = RasterFile::Create(resolvedName);
    if (rasterFilePtr == nullptr)
        {
        // Can't create model; probably that file name is invalid.
        return nullptr;
        }

    RasterFileProperties props;
    props.m_fileId = params.m_fileId;

    if (params.m_sourceToWorldP != nullptr)
        props.m_sourceToWorld = *params.m_sourceToWorldP;
    else
        {
        if (SUCCESS != ComputeGeoLocationFromFile(props.m_sourceToWorld, *rasterFilePtr, params.m_dgndb))
            return nullptr;
        }
    
    // Create model in DgnDb
    RasterFileModelPtr model = new RasterFileModel(params, props);

    return model;
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
    DEBUG_PRINTF("RasterFileModel Destroyed");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
BentleyStatus RasterFileModel::_Load(Dgn::Render::SystemP renderSys) const
    {
    if (m_root.IsValid() && (nullptr == renderSys || m_root->GetRenderSystem() == renderSys))
        return SUCCESS;

    // Resolve raster name
    BeFileName fileName;
    BentleyStatus status = T_HOST.GetRasterAttachmentAdmin()._ResolveFileName(fileName, m_fileProperties.m_fileId, GetDgnDb());
    if (status != SUCCESS)
        return ERROR;        

    Utf8String resolvedName(fileName);

    RasterSourcePtr pSource = RasterFileSource::Create(resolvedName);
    if (pSource.IsValid())
        m_root = new RasterRoot(*pSource, const_cast<RasterFileModel&>(*this), renderSys);
        
    return m_root.IsValid() ? SUCCESS : ERROR;
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

