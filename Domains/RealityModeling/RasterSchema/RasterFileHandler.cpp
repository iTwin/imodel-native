/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <RasterInternal.h>
#include <Raster/RasterFileHandler.h>
#include "RasterTileTree.h"
#include "RasterFileSource.h"
#include "GcsUtils.h"

HANDLER_DEFINE_MEMBERS(RasterFileModelHandler)

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTER


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
//------------------------------  RasterFileModelHandler  --------------------------------
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2015
//----------------------------------------------------------------------------------------
StatusInt RasterFileModelHandler::ComputeGeoLocationFromFile(DMatrix4dR sourceToWorld, RasterFileR raster, DgnDbR dgndb)
    {
    GeoCoordinates::BaseGCSPtr pSourceGcs = raster.GetBaseGcs();

    DgnGCSP pDgnGcs = dgndb.GeoLocation().GetDgnGCS();

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
            /* unused - ReprojectStatus status = REPROJECT_Success;*/

            DPoint3d pointPixel = DPoint3d::FromInterpolateBilinear(physicalCorners[0], physicalCorners[1], physicalCorners[2], physicalCorners[3], seed[x], seed[y]);

            DPoint3d srcCartesian;
            sourceToMeter.MultiplyAndRenormalize(srcCartesian, pointPixel);

            // Transform to GCS native units.
            srcCartesian.Scale(pSourceGcs->UnitsFromMeters());

            DPoint3d bimPoint;
            if (SUCCESS != GcsUtils::Reproject(bimPoint, *pDgnGcs, srcCartesian, *pSourceGcs))
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
    if (!params.m_link->GetElementId().IsValid())        // link must be persisted.
        return nullptr;

    DMatrix4d sourceToWorld; 

    if (params.m_sourceToWorldP != nullptr)
        sourceToWorld = *params.m_sourceToWorldP;
    else
        {
        // // Find resolved file name for the raster
        // BeFileName fileName;
        // BentleyStatus status = T_HOST.GetRasterAttachmentAdmin()._ResolveFileUri(fileName, params.m_link->GetUrl(), params.m_dgndb);
        // if (status != SUCCESS)
        //     return nullptr;
            
        // // Open raster 
        // //&&MM &&ep We must restructure that Create so rasterFilePtr is not lost but somehow reuse as input parameter to RasterFileModel.  
        // // Otherwise the raster file is opened twice and this might be slow for network connection.
        // RasterFilePtr rasterFilePtr = RasterFile::Create(fileName.GetNameUtf8());
        // if (rasterFilePtr == nullptr)
        //     {
        //     // Can't create model; probably that file name is invalid.
        //     return nullptr;
        //     }

        // if (SUCCESS != ComputeGeoLocationFromFile(sourceToWorld, *rasterFilePtr, params.m_dgndb))
        //     return nullptr;
        }
    
    // Create model in DgnDb
    RasterFileModelPtr model = new RasterFileModel(params, sourceToWorld);

    return model;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params) 
:T_Super (params)
    {
    m_sourceToWorld.InitIdentity();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::RasterFileModel(CreateParams const& params, DMatrix4dCR sourceToWorld)
:T_Super (params)
    {
    m_sourceToWorld = sourceToWorld;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterFileModel::~RasterFileModel()
    {
    //DEBUG_PRINTF("RasterFileModel Destroyed");
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
Dgn::Cesium::RootPtr RasterFileModel::_CreateCesiumTileTree(Dgn::Cesium::OutputR output)
    {
    if (m_loadFileFailed)   // We already tried and failed to open the file. do not try again.
        return nullptr;

    RefCountedCPtr<RepositoryLink> pLink = ILinkElementBase<RepositoryLink>::Get(GetDgnDb(), GetModeledElementId());
    if (!pLink.IsValid())
        {
        m_loadFileFailed = false;
        return nullptr;
        }

    // // Resolve raster name
    // BeFileName fileName;
    // BentleyStatus status = T_HOST.GetRasterAttachmentAdmin()._ResolveFileUri(fileName, pLink->GetUrl(), GetDgnDb());
    // if (status != SUCCESS)
    //     {
    //     m_loadFileFailed = true;
    //     return nullptr;
    //     }

    // RasterRootPtr rasterRoot = RasterFileSource::Create(fileName.GetNameUtf8(), const_cast<RasterFileModel&>(*this));
    // if (rasterRoot.IsNull())
    //     m_loadFileFailed = true;
         
    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();

    Json::Value val;
    DMatrix4dToJson(val, m_sourceToWorld);
    SetJsonProperties(json_srcToDb(), val);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterFileModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();
    DMatrix4dFromJson(m_sourceToWorld, GetJsonProperties(json_srcToDb()));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
bool RasterFileModel::_IsParallelToGround() const
    {
    DMatrix4dCR srcToWlrd = GetSourceToWorld();
    DVec3d uvCross = DVec3d::FromCrossProduct(srcToWlrd.coff[0][0], srcToWlrd.coff[1][0], srcToWlrd.coff[2][0],
                                              srcToWlrd.coff[0][1], srcToWlrd.coff[1][1], srcToWlrd.coff[2][1]);

    return uvCross.IsParallelTo(DVec3d::From(0, 0, 1));
    }

