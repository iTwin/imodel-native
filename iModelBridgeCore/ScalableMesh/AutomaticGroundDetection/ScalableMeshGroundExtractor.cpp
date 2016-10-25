/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/ScalableMeshGroundExtractor.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ScalableMeshGroundExtractor.h"
#include "ScalableMeshPointsProvider.h"
#include "..\STM\ScalableMesh.h"

#include <TerrainModel\AutomaticGroundDetection\GroundDetectionMacros.h>
#include <TerrainModel\AutomaticGroundDetection\GroundDetectionManager.h>
#include <TerrainModel\AutomaticGroundDetection\IGroundDetectionServices.h>
#include <TerrainModel\AutomaticGroundDetection\IPointsAccumulator.h>

#include <ScalableMesh\IScalableMeshSourceCreator.h>
#include <ScalableMesh\IScalableMeshSources.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshTextureGenerator.h>
#include <ScalableMesh/ScalableMeshLib.h>

#include <Bentley\BeDirectoryIterator.h>

USING_NAMESPACE_GROUND_DETECTION

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

static int s_xyzId = 0;

BeFileName GetTempXyzFilePath()
    {
    
    BeFileName tempPath;
    BeFileNameStatus status = BeFileName::BeGetTempPath(tempPath);
    assert(status == BeFileNameStatus::Success);

    wchar_t bufferId[10];
    _swprintf(bufferId, L"%i", s_xyzId);
    tempPath.AppendToPath(L"detectGround");
    tempPath.AppendString(bufferId);    
    tempPath.AppendString(L".xyz");
    return tempPath;
    }

/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor - Begin
+----------------------------------------------------------------------------*/
IScalableMeshGroundExtractorPtr IScalableMeshGroundExtractor::Create(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh)
    {
    IScalableMeshGroundExtractorPtr groundExtractor(ScalableMeshGroundExtractor::Create(smTerrainPath, scalableMesh).get());
    return groundExtractor;
    }

StatusInt IScalableMeshGroundExtractor::ExtractAndEmbed(const BeFileName& coverageTempDataFolder)
    {
    return _ExtractAndEmbed(coverageTempDataFolder);
    }        

StatusInt IScalableMeshGroundExtractor::SetExtractionArea(const bvector<DPoint3d>& area)
    {
    return _SetExtractionArea(area);
    }        

void IScalableMeshGroundExtractor::GetTempDataLocation(BeFileName& textureSubFolderName, BeFileName& extraLinearFeatureFileName)
    {
    textureSubFolderName = BeFileName(L"\\Textures\\");    
    extraLinearFeatureFileName = BeFileName(L"CoverageBreaklines.dat");        
    }        

/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor Method Definition Section - End
+----------------------------------------------------------------------------*/
struct ScalableMeshPointsAccumulator : public IGroundPointsAccumulator
    {
    private : 

        FILE* m_xyzFile;

    protected : 

        void _AddPoints(const bvector<DPoint3d>& points) override
            {
            char buffer[1000];

            for (auto& point : points)
                {
                int nbChars = sprintf(buffer, 
                                     "%.20f,%.20f,%.20f\r\n", 
                                     point.x, 
                                     point.y,
                                     point.z); 

                fwrite(buffer, nbChars, 1, m_xyzFile);
                }

            fflush(m_xyzFile);
            }

    public :

        ScalableMeshPointsAccumulator()
            {            
            BeFileName xyzFile(GetTempXyzFilePath());
            m_xyzFile = _wfopen(xyzFile.c_str(), L"w+");
            }

        ~ScalableMeshPointsAccumulator()
            {
            fclose(m_xyzFile);
            }

        void Close()
            {
            fclose(m_xyzFile);
            }

    };



ScalableMeshGroundExtractorPtr ScalableMeshGroundExtractor::Create(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh)
    {
    return new ScalableMeshGroundExtractor(smTerrainPath, scalableMesh);
    }

ScalableMeshGroundExtractor::ScalableMeshGroundExtractor(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh)
    {
    m_scalableMesh = scalableMesh;
    m_smTerrainPath = smTerrainPath;
    }

ScalableMeshGroundExtractor::~ScalableMeshGroundExtractor()
    {
    }

static bool s_createTexture = true; 
static double s_pixelSize = 1;
static DRange3d s_availableRange;
/*

void ScalableMeshGroundExtractor::GetColor(uint8_t* currentColor, const DRay3d& ray, IScalableMeshMeshQueryPtr& meshQueryInterface)
    {
    bvector<IScalableMeshNodePtr> nodes;
    
    bvector<DPoint3d> points(5); 
    points[0].x = origin.x - s_pixelSize;
    points[0].y = origin.y - s_pixelSize;
    points[0].z = origin.z;

    points[1].x = origin.x - s_pixelSize;
    points[1].y = origin.y + s_pixelSize;
    points[1].z = origin.z;

    points[2].x = origin.x + s_pixelSize;
    points[2].y = origin.y + s_pixelSize;
    points[2].z = origin.z;

    points[3].x = origin.x + s_pixelSize;
    points[3].y = origin.y - s_pixelSize;
    points[3].z = origin.z;

    points[4].x = origin.x - s_pixelSize;
    points[4].y = origin.y - s_pixelSize;
    points[4].z = origin.z;

    meshQueryInterface->Query(nodes, points, 5, params);

    for (auto& node : nodes)
        {        
        if (node->IntersectRay(intersectPointTemp, ray, valTemp))
            {
            double param;
            DPoint3d pPt;
            if (ray.ProjectPointUnbounded(pPt, param, intersectPointTemp) && param < minParam)
                {
                minParam = param;
                val = valTemp;
                intersectPoint = intersectPointTemp;
                }
            }
        }
        

    }    

StatusInt ScalableMeshGroundExtractor::CreateAndAddTexture(IDTMSourceCollection& terrainSources)
    {
    uint32_t nbPixelsX = ceil(s_availableRange.XLength() / s_pixelSize);
    uint32_t nbPixelsY = ceil(s_availableRange.YLength() / s_pixelSize);

    double stepX = s_availableRange.XLength() / nbPixelsX;
    double stepY = s_availableRange.YLength() / nbPixelsY;

    uint8_t* imagesBuffer = new uint8_t[nbPixelsX * nbPixelsY];

    double currentX = s_availableRange.low.x;
    uint8_t currentColor[3];
    DVec3d direction(DVec3d::From(0, 0, -1));

    IScalableMeshMeshQueryPtr meshQueryInterface = smDesign->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    params->SetLevel(smDesign->GetTerrainDepth());

            
    for (size_t xInd = 0; xInd < nbPixelsX; xInd++)
        {
        double currentY = s_availableRange.low.y;

        for (size_t yInd = 0; yInd < nbPixelsX; yInd++)
            {
            DPoint3d origin(DPoint3d::From(currentX, currentY, s_availableRange.high.z * 2));
            DRay3d ray = DRay3d::FromOriginAndVector(origin, direction);
            GetColor(currentColor, ray);
            currentY += stepY;
            }

        currentX += stepX;
        }
    }
    */

static bool s_deactivateForMultiCoverage = false;

StatusInt ScalableMeshGroundExtractor::CreateSmTerrain(const BeFileName& coverageTempDataFolder)
    {
    StatusInt status;
            
    IScalableMeshSourceCreatorPtr terrainCreator(IScalableMeshSourceCreator::GetFor(m_smTerrainPath.c_str(), status));

    assert(status == SUCCESS);
    auto editFilesString = ((ScalableMeshBase*)m_scalableMesh.get())->GetPath();
    terrainCreator->SetBaseExtraFilesPath(m_smTerrainPath);
    if (m_scalableMesh->GetBaseGCS().IsValid())
        status = terrainCreator->SetBaseGCS(m_scalableMesh->GetBaseGCS());

    assert(status == SUCCESS);

    BeFileName xyzFile(GetTempXyzFilePath());

    IDTMLocalFileSourcePtr groundPtsSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_POINT, xyzFile.c_str()));
    SourceImportConfig& sourceImportConfig = groundPtsSource->EditConfig();
    Import::ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

    data.SetRepresenting3dData(false);

    sourceImportConfig.SetReplacementSMData(data);
    terrainCreator->EditSources().Add(groundPtsSource);

    //Add texture if any    
    BeFileName currentTextureDir(coverageTempDataFolder);
    BeFileName textureSubFolderName;
    BeFileName extraLinearFeatureFileName;

    IScalableMeshGroundExtractor::GetTempDataLocation(textureSubFolderName, extraLinearFeatureFileName);        

    currentTextureDir.AppendString(textureSubFolderName.c_str());    

    IScalableMeshTextureGeneratorPtr textureGenerator(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetTextureGenerator());

    assert(textureGenerator.IsValid());

    textureGenerator->SetPixelSize(0.10);
    textureGenerator->SetTextureTempDir(currentTextureDir);

    DRange3d covExt = DRange3d::From(m_extractionArea);

    if (!s_deactivateForMultiCoverage)
        {
        bvector<bvector<DPoint3d>> polys;
        m_scalableMesh->GetAllCoverages(polys);

        for (auto& poly : polys)
            {
            DRange3d newRange = DRange3d::From(poly);
            covExt.Extend(newRange);
            }
        }

    
    covExt.ScaleAboutCenter(covExt, 1.1);

     if (!s_deactivateForMultiCoverage)
        {
        bvector<DPoint3d> closedPolygonPoints;
        DPoint3d rangePts[5] = { DPoint3d::From(covExt.low.x, covExt.low.y, 0), DPoint3d::From(covExt.low.x, covExt.high.y, 0), DPoint3d::From(covExt.high.x, covExt.high.y, 0),
            DPoint3d::From(covExt.high.x, covExt.low.y, 0), DPoint3d::From(covExt.low.x, covExt.low.y, 0) };
        closedPolygonPoints.assign(rangePts, rangePts + 5);

        textureGenerator->GenerateTexture(closedPolygonPoints);
        }

    BeDirectoryIterator directoryIter(currentTextureDir);

    BeFileName currentTextureName;
    bool       isDir;            
    while (SUCCESS == directoryIter.GetCurrentEntry (currentTextureName, isDir))
        {        
        if (0 == currentTextureName.GetExtension().CompareToI(L"jpg"))
            {
            IDTMLocalFileSourcePtr textureSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_IMAGE, currentTextureName.c_str()));            
            terrainCreator->EditSources().Add(textureSource);                       
            }        

        directoryIter.ToNext();
        }

    BeFileName coverageBreaklineFile(coverageTempDataFolder);
    coverageBreaklineFile.AppendString(L"\\");    
    coverageBreaklineFile.AppendString(extraLinearFeatureFileName.c_str());    
    
    if (coverageBreaklineFile.DoesPathExist())
        {
        if (!s_deactivateForMultiCoverage)
            {
            IDTMLocalFileSourcePtr coverageBreaklineSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_BREAKLINE, coverageBreaklineFile.c_str()));
            terrainCreator->EditSources().Add(coverageBreaklineSource);                       
            }        
        }

    status = terrainCreator->Create();
    assert(status == SUCCESS);
    terrainCreator->SaveToFile();
    terrainCreator = nullptr;

    s_xyzId++;

    if (!s_deactivateForMultiCoverage)
        {    
        int result = _wremove(xyzFile.c_str());
        assert(result == 0);
        }

    return status;
    }

//#define LARGEST_STRUCTURE_SIZE_DEFAULT 60 
#define LARGEST_STRUCTURE_SIZE_DEFAULT 30 

StatusInt ScalableMeshGroundExtractor::_ExtractAndEmbed(const BeFileName& coverageTempDataFolder)
    {    
    IGroundDetectionServices* serviceP(GroundDetectionManager::GetServices());

    bvector<DPoint3d> seedpoints;    
    GroundDetectionParametersPtr params(GroundDetectionParameters::Create());        
    params->SetLargestStructureSize(LARGEST_STRUCTURE_SIZE_DEFAULT);
    params->SetTriangleEdgeThreshold(0.05);
 
    params->SetAnglePercentileFactor(30);
    params->SetHeightPercentileFactor(60);

    ScalableMeshPointsProviderCreatorPtr smPtsProviderCreator(ScalableMeshPointsProviderCreator::Create(m_scalableMesh));    
    smPtsProviderCreator->SetExtractionArea(m_extractionArea);

    DRange3d availableRange;
    smPtsProviderCreator->GetAvailableRange(availableRange);

    s_availableRange = availableRange;

    double maxLength = std::max(availableRange.XLength(), availableRange.YLength());

    if (maxLength < LARGEST_STRUCTURE_SIZE_DEFAULT * 2)
        {
        params->SetLargestStructureSize(maxLength / 2);
        }
    
    IPointsProviderCreatorPtr ptsProviderCreator(smPtsProviderCreator.get());     
    params->SetPointsProviderCreator(ptsProviderCreator);        

    IGroundPointsAccumulatorPtr accumPtr(new ScalableMeshPointsAccumulator());    

    params->SetGroundPointsAccumulator(accumPtr);

    /*
    StatusInt status = serviceP->_GetSeedPointsFromTIN(seedpoints, *params.get());
    assert(status == SUCCESS);
    */
    
    StatusInt status = serviceP->_DoGroundDetection(*params.get());
    assert(status == SUCCESS);

    IGroundPointsAccumulatorPtr nullAcc;

    params->SetGroundPointsAccumulator(nullAcc);
    accumPtr = 0;

    status = CreateSmTerrain(coverageTempDataFolder);
    
    return status;        
    } 

StatusInt ScalableMeshGroundExtractor::_SetExtractionArea(const bvector<DPoint3d>& area) 
    {
    m_extractionArea.insert(m_extractionArea.end(), area.begin(), area.end());
    return SUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
