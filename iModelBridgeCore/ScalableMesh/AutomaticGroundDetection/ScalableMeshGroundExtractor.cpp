/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ScalableMeshGroundExtractor.h"
#include "ScalableMeshPointsProvider.h"
#include "../STM/ScalableMesh.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/GroundDetectionManager.h>
#include <TerrainModel/AutomaticGroundDetection/IGroundDetectionServices.h>
#include <TerrainModel/AutomaticGroundDetection/IPointsAccumulator.h>

#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSources.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshTextureGenerator.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <ScalableMesh/IScalableMeshPolicy.h>

#include "../GeoCoords/ReprojectionUtils.h"


#include "../STM/GeneratorTextureProvider.h"

#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeConsole.h>

#ifdef VANCOUVER_API
#include <DgnGeoCoord/DgnGeoCoord.h>
#else
#include <DgnPlatform/DgnGeoCoord.h>
#endif

#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_GROUND_DETECTION

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

static int s_xyzId = 0;

BeFileName GetTempXyzFilePath()
    {    
    BeFileName tempPath;

#ifdef VANCOUVER_API
    BeFileNameStatus status = BeFileName::BeGetTempPath(tempPath);
    assert(status == BeFileNameStatus::Success);
#else
    DgnPlatformLib::Host::IKnownLocationsAdmin& locationAdmin(DgnPlatformLib::QueryHost()->GetIKnownLocationsAdmin());
    tempPath = locationAdmin.GetLocalTempDirectoryBaseName();
    assert(!tempPath.IsEmpty());
#endif
    
    wchar_t bufferId[10];
    _swprintf(bufferId, L"%i", s_xyzId);
    tempPath.AppendToPath(L"detectGround");
    tempPath.AppendString(bufferId);    
    tempPath.AppendString(L".xyz");
    return tempPath;
    }

/*----------------------------------------------------------------------------+
|IScalableMeshGroundPreviewer - Begin
+----------------------------------------------------------------------------*/
bool IScalableMeshGroundPreviewer::IsCurrentPreviewEnough() const
    {
    return _IsCurrentPreviewEnough();
    }

StatusInt IScalableMeshGroundPreviewer::UpdatePreview(PolyfaceQueryCR currentGround)
    {
    return _UpdatePreview(currentGround);
    }

bool IScalableMeshGroundPreviewer::UpdateProgress(IScalableMeshProgress* progress)
    {
		return _UpdateProgress(progress);
    }
/*----------------------------------------------------------------------------+
|IScalableMeshGroundPreviewer - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor - Begin
+----------------------------------------------------------------------------*/
IScalableMeshGroundExtractorPtr IScalableMeshGroundExtractor::Create(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh)
    {
    IScalableMeshGroundExtractorPtr groundExtractor(ScalableMeshGroundExtractor::Create(smTerrainPath, scalableMesh).get());
    return groundExtractor;
    }

SMStatus IScalableMeshGroundExtractor::ExtractAndEmbed(const BeFileName& coverageTempDataFolder)
    {
    return _ExtractAndEmbed(coverageTempDataFolder);
    }        

StatusInt IScalableMeshGroundExtractor::SetDestinationGcs(GeoCoordinates::BaseGCSPtr& destinationGcs)
    {
    return _SetDestinationGcs(destinationGcs);
    }

StatusInt IScalableMeshGroundExtractor::SetExtractionArea(const bvector<DPoint3d>& area)
    {
    return _SetExtractionArea(area);
    }  

StatusInt IScalableMeshGroundExtractor::SetDataSourceDir(const BeFileName& dataSourceDir) 
    {
    return _SetDataSourceDir(dataSourceDir);
    }  

StatusInt IScalableMeshGroundExtractor::SetReprojectElevation(bool doReproject) 
    {
    return _SetReprojectElevation(doReproject);
    }

StatusInt IScalableMeshGroundExtractor::SetLimitTextureResolution(bool limitTextureResolution)
{
	return _SetLimitTextureResolution(limitTextureResolution);
}

StatusInt IScalableMeshGroundExtractor::SetGroundPreviewer(IScalableMeshGroundPreviewerPtr& groundPreviewer)
    {
    return _SetGroundPreviewer(groundPreviewer);
    }

void IScalableMeshGroundExtractor::GetTempDataLocation(BeFileName& textureSubFolderName, BeFileName& extraLinearFeatureFileName)
    {
    textureSubFolderName = BeFileName(L"\\Textures\\");    
    extraLinearFeatureFileName = BeFileName(L"CoverageBreaklines");
    extraLinearFeatureFileName.append(std::to_wstring(s_xyzId).c_str());
    extraLinearFeatureFileName.append(L".dat");        
    }        


bool s_shouldStopContinueEarly = false;

/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor Method Definition Section - End
+----------------------------------------------------------------------------*/
struct ScalableMeshPointsAccumulator : public IGroundPointsAccumulator
    {
    private : 

        FILE*                           m_xyzFile;
        size_t                          m_nbPoints;
        IScalableMeshGroundPreviewerPtr m_groundPreviewer;
        Transform                       m_previewTransform;

        GeoCoordInterpretation m_geocoordInterpretation;

        BaseGCSPtr             m_sourceGcs;
        BaseGCSPtr             m_destinationGcs;

        inline void Reproject(DPoint3d& ptOut,const DPoint3d& ptIn)
            {
            if (m_destinationGcs.IsValid())
                {
                assert(m_sourceGcs.IsValid());
                GeoPoint srcLatLong;
                GeoPoint dstLatLong;                

                if (m_geocoordInterpretation == GeoCoordInterpretation::Cartesian)
                    {
                    m_sourceGcs->LatLongFromCartesian(srcLatLong, ptIn);
                    }
                else
                    {
                    m_sourceGcs->LatLongFromXYZ(srcLatLong, ptIn);
                    }

                m_sourceGcs->LatLongFromLatLong(dstLatLong, srcLatLong, *m_destinationGcs);

                m_destinationGcs->CartesianFromLatLong(ptOut, dstLatLong);
                }
            else
                {
                ptOut = ptIn;
                }
            }
        

    protected : 

        virtual void _AddPoints(const bvector<DPoint3d>& points) override
            {
            char buffer[1000];

            for (auto& point : points)
                {
                DPoint3d reprojPoint;
                Reproject(reprojPoint, point);
                
                int nbChars = sprintf(buffer, 
                                     "%.20f,%.20f,%.20f\r\n", 
                                     reprojPoint.x,
                                     reprojPoint.y,
                                     reprojPoint.z);

                fwrite(buffer, nbChars, 1, m_xyzFile);
                }

            fflush(m_xyzFile);

            m_nbPoints += points.size();
            }

        virtual void _GetPreviewTransform(Transform& transform) const
            {
            transform = m_previewTransform;
            }

        virtual void _OutputPreview(PolyfaceQueryCR currentGround) const override
            {
            if (m_groundPreviewer.IsValid())
                m_groundPreviewer->UpdatePreview(currentGround);
            }

        virtual bool _ShouldContinue() const override
            {
            if (s_shouldStopContinueEarly)
                return false;

            if (m_groundPreviewer.IsValid())
                return !m_groundPreviewer->IsCurrentPreviewEnough();

            return true;
            }        

    public :

        ScalableMeshPointsAccumulator(IScalableMeshGroundPreviewerPtr& groundPreviewer, Transform previewTransform)
            {            
            BeFileName xyzFile(GetTempXyzFilePath());
            m_xyzFile = _wfopen(xyzFile.c_str(), L"w+");
            m_nbPoints = 0;
            m_groundPreviewer = groundPreviewer;
            m_previewTransform = previewTransform;
            }

        ~ScalableMeshPointsAccumulator()
            {
            fclose(m_xyzFile);
            }

        void SetReprojGCS(GeoCoordInterpretation geocoordInterpretation, BaseGCSPtr& sourceGcs, BaseGCSPtr& destinationGcs)
            {
            m_geocoordInterpretation = geocoordInterpretation;
            m_sourceGcs = sourceGcs;
            m_destinationGcs = destinationGcs;
            }
        
        void Close()
            {
            fclose(m_xyzFile);
            }

        size_t GetNbPoints() const 
            {
            return m_nbPoints;
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
	m_limitTextureResolution = false;
    m_reprojectElevation = false;    

    const GeoCoords::GCS& gcs(m_scalableMesh->GetGCS());    
    m_smGcsRatioToMeter = m_scalableMesh->IsCesium3DTiles() ? 1.0 : gcs.GetUnit().GetRatioToBase();
    }

ScalableMeshGroundExtractor::~ScalableMeshGroundExtractor()
    {
    }

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

static bool s_deactivateTexturing = false;
#define DEFAULT_TEXTURE_RESOLUTION 0.05

double ScalableMeshGroundExtractor::ComputeTextureResolution()
    {
    IScalableMeshMeshQueryPtr meshQueryInterface = m_scalableMesh->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    params->SetLevel(m_scalableMesh->GetNbResolutions() - 1);
       
    bvector<IScalableMeshNodePtr> nodes;
    meshQueryInterface->Query(nodes, m_extractionArea.begin(), (int)m_extractionArea.size(), params);

    double minTextureResolution = DBL_MAX;
    float  geometricResolution;
    float  textureResolution;

    for (auto& node : nodes)
        {
        node->GetResolutions(geometricResolution, textureResolution);        
        if (textureResolution != 0)
            {
            minTextureResolution = std::min(minTextureResolution, (double)textureResolution);
            }
        }

	DRange3d extractionRange = DRange3d::From(m_extractionArea);
	double targetResolutionThreshold = 0.01;//1 cm max
    
    if (m_smGcsRatioToMeter != 0)
        {
        targetResolutionThreshold /= m_smGcsRatioToMeter;
        }
        
    if (minTextureResolution != DBL_MAX)
        return  m_limitTextureResolution ? std::max(targetResolutionThreshold,minTextureResolution) * m_smGcsRatioToMeter
		: minTextureResolution * m_smGcsRatioToMeter;

    return DEFAULT_TEXTURE_RESOLUTION * m_smGcsRatioToMeter;
    }


SMStatus ScalableMeshGroundExtractor::CreateSmTerrain(const BeFileName& coverageTempDataFolder)
    {
	SMStatus status = SMStatus::S_SUCCESS;

    BeFileName terrainPath(m_smTerrainPath.c_str());
    BeFileName directory(BeFileName::GetDirectoryName(terrainPath.c_str()).c_str());

    if (!BeFileName::DoesPathExist(directory.c_str()))
        {
        BeFileNameStatus dirCreateStatus = BeFileName::CreateNewDirectory(directory.c_str());
        assert(BeFileNameStatus::Success == dirCreateStatus);
        }            
            
	StatusInt statusOpen;
    IScalableMeshSourceCreatorPtr terrainCreator(IScalableMeshSourceCreator::GetFor(m_smTerrainPath.c_str(), statusOpen));

    assert(statusOpen == SUCCESS);
    //auto editFilesString = ((ScalableMeshBase*)m_scalableMesh.get())->GetPath();
    m_createProgress.ProgressStep() = ScalableMeshStep::STEP_GENERATE_TEXTURE;
    m_createProgress.ProgressStepIndex() = 1;
    m_createProgress.Progress() = 0.0f;
    
    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(&m_createProgress);
                                               
    if (m_destinationGcs.IsValid())
        {
        status = terrainCreator->SetBaseGCS(m_destinationGcs) == SUCCESS ? SMStatus::S_SUCCESS : SMStatus::S_ERROR;
        }
    else  //NEEDS_WORK_SM : Cesium 3D tile is creating ECEF LL84 GCS, which cannot be represented at the file level currently.
    if (m_scalableMesh->GetBaseGCS().IsValid() && !m_scalableMesh->IsCesium3DTiles())
        status = terrainCreator->SetBaseGCS(m_scalableMesh->GetBaseGCS()) == SUCCESS ? SMStatus::S_SUCCESS : SMStatus::S_ERROR;

    assert(status == SMStatus::S_SUCCESS);

    BaseGCSCPtr destinationGcsPtr(terrainCreator->GetBaseGCS());
    const GCS& gcs(terrainCreator->GetGCS());
        
    BeFileName xyzFile(GetTempXyzFilePath());
    BeFileName xyzSourceFile(xyzFile); 

    if (!m_dataSourceDir.empty())
        {
        assert(BeFileName::DoesPathExist(m_dataSourceDir.c_str()));
        BeFileName newXyzFileName(m_dataSourceDir); 
        newXyzFileName.AppendString(BeFileName::GetFileNameAndExtension(xyzFile.c_str()).c_str());
                    
        BeFileNameStatus statusCopy = BeFileName::BeCopyFile(xyzFile.c_str(), newXyzFileName.c_str());
        assert(statusCopy == BeFileNameStatus::Success);

        xyzSourceFile = newXyzFileName;
        }

    IDTMLocalFileSourcePtr groundPtsSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_POINT, xyzSourceFile.c_str()));
     
    SourceImportConfig& sourceImportConfig = groundPtsSource->EditConfig();
    Import::ScalableMeshData data = sourceImportConfig.GetReplacementSMData();

    data.SetRepresenting3dData(false);

    sourceImportConfig.SetReplacementSMData(data);
    sourceImportConfig.SetReplacementGCS(gcs, true, false, false);
     

    terrainCreator->EditSources().Add(groundPtsSource);    

    //Add texture if any        
    BeFileName currentTextureDir(coverageTempDataFolder);
    BeFileName textureSubFolderName;
    BeFileName extraLinearFeatureFileName;

    IScalableMeshGroundExtractor::GetTempDataLocation(textureSubFolderName, extraLinearFeatureFileName);        

    currentTextureDir.AppendString(textureSubFolderName.c_str());    

    IScalableMeshTextureGeneratorPtr textureGenerator(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetTextureGenerator());

    if (!s_deactivateTexturing && textureGenerator.IsValid())
        {        
        assert(textureGenerator.IsValid());
        
        textureGenerator->SetPixelSize(ComputeTextureResolution());
        textureGenerator->SetTextureTempDir(currentTextureDir);
        textureGenerator->SetTransform(m_scalableMesh->GetReprojectionTransform());

        DRange3d covExt = DRange3d::From(m_extractionArea);
        covExt.ScaleAboutCenter(covExt, 1.1);

        bvector<DPoint3d> closedPolygonRangePoints(8);
        covExt.Get8Corners(closedPolygonRangePoints.data());

        if (m_createProgress.IsCanceled()) return SMStatus::S_ERROR_CANCELED_BY_USER;;

        textureGenerator->GenerateTexture(closedPolygonRangePoints, &m_createProgress);

        BeDirectoryIterator directoryIter(currentTextureDir);

        BeFileName currentTextureName;
        bool       isDir;            
        while (SUCCESS == directoryIter.GetCurrentEntry (currentTextureName, isDir))
            {        
#ifndef VANCOUVER_API
            if (0 == currentTextureName.GetExtension().CompareToI(L"jpg") || 
                0 == currentTextureName.GetExtension().CompareToI(L"itiff64"))
#else
            if (0 == BeFileName::GetExtension(currentTextureName.c_str()).CompareToI(L"jpg") || 
                0 == BeFileName::GetExtension(currentTextureName.c_str()).CompareToI(L"itiff64"))
#endif
                {
                if (!m_dataSourceDir.empty())
                    {
                    assert(BeFileName::DoesPathExist(m_dataSourceDir.c_str()));
                    BeFileName newTextureName(directory); 
                    newTextureName.AppendString(BeFileName::GetFileNameAndExtension(currentTextureName.c_str()).c_str());
                    
                    BeFileNameStatus statusCopy = BeFileName::BeCopyFile(currentTextureName.c_str(), newTextureName.c_str());
                    assert(statusCopy == BeFileNameStatus::Success);

                    currentTextureName = newTextureName;                
                    }

                IDTMLocalFileSourcePtr textureSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_IMAGE, currentTextureName.c_str()));            
                textureSource->EditConfig().SetReplacementGCS(gcs, true, false, false);
                
                terrainCreator->EditSources().Add(textureSource);                       
                }        

            directoryIter.ToNext();
            }
        }


    m_createProgress.Progress() = 1.0f;
    
    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(&m_createProgress);

    if (m_createProgress.IsCanceled()) return SMStatus::S_ERROR_CANCELED_BY_USER;

    BeFileName coverageBreaklineFile(coverageTempDataFolder);
    coverageBreaklineFile.AppendString(L"\\");    
    coverageBreaklineFile.AppendString(extraLinearFeatureFileName.c_str());    

#ifndef VANCOUVER_API    
    if (coverageBreaklineFile.DoesPathExist())
#else
    if (BeFileName::DoesPathExist(coverageBreaklineFile.c_str()))
#endif     
        {        
        if (!m_dataSourceDir.empty())
            {
            assert(BeFileName::DoesPathExist(m_dataSourceDir.c_str()));
            BeFileName newCoverageBreaklineFileName(m_dataSourceDir); 
            newCoverageBreaklineFileName.AppendString(BeFileName::GetFileNameAndExtension(coverageBreaklineFile.c_str()).c_str());
                    
            BeFileNameStatus statusCopy = BeFileName::BeCopyFile(coverageBreaklineFile.c_str(), newCoverageBreaklineFileName.c_str());
            assert(statusCopy == BeFileNameStatus::Success);

            coverageBreaklineFile = newCoverageBreaklineFileName;                
            }

        IDTMLocalFileSourcePtr coverageBreaklineSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_BREAKLINE, coverageBreaklineFile.c_str()));
        coverageBreaklineSource->EditConfig().SetReplacementGCS(gcs, true, false, false);
        
        terrainCreator->EditSources().Add(coverageBreaklineSource); 
        }

    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(terrainCreator->GetProgress());
    
    status = terrainCreator->Create();
    terrainCreator->SaveToFile();

	if (terrainCreator->GetProgress()->IsCanceled())
		return SMStatus::S_ERROR_CANCELED_BY_USER;
    
    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(nullptr);
    terrainCreator = nullptr;


#if 0
    StatusInt openStatus;

    IScalableMeshPtr smP = IScalableMesh::GetFor(m_smTerrainPath.c_str(), false, true, openStatus);
    IScalableMeshCreatorPtr creator(IScalableMeshCreator::GetFor(smP, status));
    DRange3d range;
    m_scalableMesh->GetRange(range);
    ITextureProviderPtr genProvider = new GeneratorTextureProvider(textureGenerator, range, 0.05, currentTextureDir);
    creator->SetTextureProvider(genProvider);
    creator = nullptr;
    smP = nullptr;
#endif

    assert(status == SMStatus::S_SUCCESS);
    s_xyzId++;
    
    int result = _wremove(xyzFile.c_str());
    assert(result == 0);    

    return status;
    }

//#define LARGEST_STRUCTURE_SIZE_DEFAULT 60 
#define LARGEST_STRUCTURE_SIZE_DEFAULT 30 
/*
static double s_anglePercentile = 30;
static double s_heightPercentile = 60;
*/

/*NEEDS_WORK_MST : For ground optimization*/
static double s_anglePercentile = 65;
static double s_heightPercentile = 65;

static bool   s_useMultiThread = true;

static double s_time;
static size_t s_nbPoints;
static bool   s_activateLog = false;

void ScalableMeshGroundExtractor::AddXYZFilePointsAsSeedPoints(GroundDetectionParametersPtr& params, const BeFileName& coverageTempDataFolder)
    {
    BeFileName textureSubFolderName;
    BeFileName extraLinearFeatureFileName;

    IScalableMeshGroundExtractor::GetTempDataLocation(textureSubFolderName, extraLinearFeatureFileName);

    BeFileName coverageBreaklineFile(coverageTempDataFolder);
    coverageBreaklineFile.AppendString(L"\\");
    coverageBreaklineFile.AppendString(extraLinearFeatureFileName.c_str());

#ifndef VANCOUVER_API    
    if (coverageBreaklineFile.DoesPathExist())
#else
    if (BeFileName::DoesPathExist(coverageBreaklineFile.c_str()))
#endif
        {
        BcDTMPtr dtmPtr(BcDTM::CreateFromGeopakDatFile(coverageBreaklineFile.c_str()));

        if (!dtmPtr.IsValid())
            return;
        
        DPoint3d pt;                
        bvector<DPoint3d> addtionalSeedPts; 

        BaseGCSPtr sourceGcs;

        if (!m_scalableMesh->GetGCS().IsNull() && m_destinationGcs.IsValid() && !m_scalableMesh->IsCesium3DTiles())
            {
            sourceGcs = BaseGCS::CreateGCS(*m_scalableMesh->GetGCS().GetGeoRef().GetBasePtr());

            if (m_reprojectElevation)
                {
                sourceGcs->SetReprojectElevation(true);
                m_destinationGcs->SetReprojectElevation(true);
                }
            }
            
        for (int ptInd = 0; ptInd < dtmPtr->GetPointCount(); ptInd++)
            {             
            DTMStatusInt status = dtmPtr->GetPoint(ptInd, pt);
            assert(status == SUCCESS);

            if (sourceGcs.IsValid())
                {
                ReprojectPt(pt, pt, m_destinationGcs, sourceGcs, GeoCoordInterpretation::Cartesian, GeoCoordInterpretation::Cartesian);
                }

            addtionalSeedPts.push_back(pt);
            }

#ifdef VANCOUVER_API
        WString envVarStr;

        if (BSISUCCESS == ConfigurationManager::GetVariable(envVarStr, L"SM_GROUND_MAX_SEEDS_FROM_DRAPED_ROI"))
            {
            int value = _wtoi(envVarStr.c_str());

            if (value > 0 && addtionalSeedPts.size() > (double)value)
                {                
                int skipIncrement = ceil(addtionalSeedPts.size() / (double)value);

                auto seedPtIter = addtionalSeedPts.begin();

                int iterInd = 0;

                while (seedPtIter != addtionalSeedPts.end())
                    {
                    if (iterInd % skipIncrement != 0)
                        seedPtIter = addtionalSeedPts.erase(seedPtIter);
                    else
                        seedPtIter++;
                    
                    iterInd++;
                    }                
                }
            }
#endif

        params->AddAdditionalSeedPoints(addtionalSeedPts);        
        }    
    }

SMStatus ScalableMeshGroundExtractor::_ExtractAndEmbed(const BeFileName& coverageTempDataFolder)
    {    
    IGroundDetectionServices* serviceP(GroundDetectionManager::GetServices());

    bvector<DPoint3d> seedpoints;    
    GroundDetectionParametersPtr params(GroundDetectionParameters::Create());        
    params->SetLargestStructureSize(LARGEST_STRUCTURE_SIZE_DEFAULT);

/*
#ifdef VANCOUVER_API //TFS# 725973 - Descartes prefers faster processing than more precise results.
    params->SetTriangleEdgeThreshold(1.0);
#else
*/
    params->SetTriangleEdgeThreshold(0.05);
//#endif

#ifdef VANCOUVER_API
    WString envVarStr;

    if (BSISUCCESS == ConfigurationManager::GetVariable(envVarStr, L"SM_GROUND_TRI_EDGE"))
        {
        double value = _wtof(envVarStr.c_str());

        if (value > 0)
            params->SetTriangleEdgeThreshold(value);
        }
#endif
	 
    params->SetAnglePercentileFactor(s_anglePercentile);
    params->SetHeightPercentileFactor(s_heightPercentile);

    params->SetUseMultiThread(s_useMultiThread);        
    

//#ifndef VANCOUVER_API //TFS# 725973 - Descartes prefers faster processing than more precise results.
    AddXYZFilePointsAsSeedPoints(params, coverageTempDataFolder);
//#endif


    m_createProgress.ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_DETECT_GROUND;
    m_createProgress.ProgressStep() = ScalableMeshStep::STEP_DETECT_GROUND;
    m_createProgress.ProgressStepIndex() = 0;
    m_createProgress.Progress() = 0.0f;

    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(&m_createProgress);

    if (m_createProgress.IsCanceled()) return SMStatus::S_ERROR_CANCELED_BY_USER;
    ScalableMeshPointsProviderCreatorPtr smPtsProviderCreator(ScalableMeshPointsProviderCreator::Create(m_scalableMesh));    

    if (!m_scalableMesh->GetGCS().IsNull() && m_destinationGcs.IsValid() && m_scalableMesh->IsCesium3DTiles())
        {
        BaseGCSPtr sourceGcs(BaseGCS::CreateGCS(*m_scalableMesh->GetGCS().GetGeoRef().GetBasePtr()));

        if (m_reprojectElevation)
            {
            sourceGcs->SetReprojectElevation(true);
            }
        

        auto coordInterp = m_scalableMesh->IsCesium3DTiles() ? GeoCoordInterpretation::XYZ : GeoCoordInterpretation::Cartesian;

        smPtsProviderCreator = ScalableMeshPointsProviderCreator::Create(m_scalableMesh, sourceGcs, m_destinationGcs, coordInterp);
        }
    else
        {
        smPtsProviderCreator = ScalableMeshPointsProviderCreator::Create(m_scalableMesh);
        }

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

    IGroundPointsAccumulatorPtr accumPtr(new ScalableMeshPointsAccumulator(m_groundPreviewer, m_scalableMesh->GetReprojectionTransform()));

    if (!m_scalableMesh->GetGCS().IsNull() && m_destinationGcs.IsValid() && !m_scalableMesh->IsCesium3DTiles())
        {                 
        auto coordInterp = m_scalableMesh->IsCesium3DTiles() ? GeoCoordInterpretation::XYZ : GeoCoordInterpretation::Cartesian;

        BaseGCSPtr sourceGcs(BaseGCS::CreateGCS(*m_scalableMesh->GetGCS().GetGeoRef().GetBasePtr()));

        if (m_reprojectElevation)
            {
            sourceGcs->SetReprojectElevation(true);
            }
        
        ((ScalableMeshPointsAccumulator*)accumPtr.get())->SetReprojGCS(coordInterp, sourceGcs, m_destinationGcs);
        }

    params->SetGroundPointsAccumulator(accumPtr);

    /*
    StatusInt status = serviceP->_GetSeedPointsFromTIN(seedpoints, *params.get());
    assert(status == SUCCESS);
    */

    clock_t startTime = clock();
    
    StatusInt status = serviceP->_DoGroundDetection(*params.get());
    m_createProgress.Progress() = 1.0f;

    if (m_groundPreviewer.IsValid())
        m_groundPreviewer->UpdateProgress(&m_createProgress);

    if (m_createProgress.IsCanceled()) return SMStatus::S_ERROR_CANCELED_BY_USER;
    assert(status == SUCCESS);

    clock_t endTime = clock() - startTime;
    double duration = (double)endTime / CLOCKS_PER_SEC;

    if (s_activateLog)
        {
        FILE* logfile = fopen("D:\\MyDoc\\RM - SM - Sprint 15\\OptimizingGroundDetection\\Log.txt","a");
        s_time = duration;
        s_nbPoints = ((ScalableMeshPointsAccumulator*)accumPtr.get())->GetNbPoints();

        if (s_useMultiThread)
            //fprintf(logfile, "Total MT ground detection executionTime : %f nbGroundPoints %I64 \r\n", duration, ((ScalableMeshPointsAccumulator*)accumPtr.get())->GetNbPoints());
            fprintf(logfile, "Total MT ground detection executionTime : %.3f nbGroundPoints %zi \r\n", duration, ((ScalableMeshPointsAccumulator*)accumPtr.get())->GetNbPoints());
        else
            fprintf(logfile, "Total ST ground detection executionTime : %.3f nbGroundPoints %zi \r\n", duration, ((ScalableMeshPointsAccumulator*)accumPtr.get())->GetNbPoints());

        fclose(logfile);
        }
        
    IGroundPointsAccumulatorPtr nullAcc;

    params->SetGroundPointsAccumulator(nullAcc);
    accumPtr = 0;

    SMStatus statusTerrain = CreateSmTerrain(coverageTempDataFolder);
    
    return statusTerrain;
    } 

static bool s_fixTest = false;


StatusInt ScalableMeshGroundExtractor::_SetDestinationGcs(GeoCoordinates::BaseGCSPtr& destinationGcs)
    {
    m_destinationGcs = destinationGcs;
    return SUCCESS;
    }

StatusInt ScalableMeshGroundExtractor::_SetLimitTextureResolution(bool limitTextureResolution)
    {
	m_limitTextureResolution = limitTextureResolution;
	return SUCCESS;
    }

StatusInt ScalableMeshGroundExtractor::_SetExtractionArea(const bvector<DPoint3d>& area) 
    {
    if (!s_fixTest)
        {                 
        Transform transform(m_scalableMesh->GetReprojectionTransform());

        if (transform.IsIdentity())
            { 
            m_extractionArea.insert(m_extractionArea.end(), area.begin(), area.end());            

            double ratioFromMeter = 1.0 / m_smGcsRatioToMeter;

            //Convert from UOR to SM unit.
            for (auto& pt : m_extractionArea)
                {
                pt.Scale(ratioFromMeter);
                }
            }            
        else
            {        
            Transform transformToSm;
            bool result = transformToSm.InverseOf(transform);
            assert(result == true);            
            transformToSm.Multiply(m_extractionArea, area);
            }               
        }
    else
        {        
        /*Small Melaka*/
        m_extractionArea.push_back(DPoint3d::From(194142.31717461502, 243656.46210683032, -3.4523928137150506));
        m_extractionArea.push_back(DPoint3d::From(194165.08343336626, 243603.61276573580, -3.6304642277846142));
        m_extractionArea.push_back(DPoint3d::From(194270.10891005833, 243574.39524382990, -4.7820067383618152));
        m_extractionArea.push_back(DPoint3d::From(194276.55219083698, 243624.66656828567, -4.6135505912207009));
        m_extractionArea.push_back(DPoint3d::From(194147.68657526391, 243662.26264426752, -3.6141459398750158));
        m_extractionArea.push_back(DPoint3d::From(194146.39791910816, 243662.47747898742, -3.6220131969348586));
        m_extractionArea.push_back(DPoint3d::From(194142.31717461502, 243656.46210683032, -3.4523928137150506));               

        /*Big Melaka*/
        /*
        m_extractionArea.push_back(DPoint3d::From(194534.61905953390, 243398.93278513860, -5.8858629763999488));
        m_extractionArea.push_back(DPoint3d::From(194475.43182124716, 243320.97185519966, -5.5277979698057607));
        m_extractionArea.push_back(DPoint3d::From(194520.70126835266, 243294.30101074686, -5.7520366628632473));
        m_extractionArea.push_back(DPoint3d::From(194549.30599180690, 243253.81847898816, -5.6788048221951613));
        m_extractionArea.push_back(DPoint3d::From(194556.74102235903, 243256.63943368991, -5.6507380919611023));
        m_extractionArea.push_back(DPoint3d::From(194566.66660501724, 243248.11899908053, -5.7024422213071375));
        m_extractionArea.push_back(DPoint3d::From(194583.99582471355, 243202.73250710749, -5.7479987083188462));
        m_extractionArea.push_back(DPoint3d::From(194592.00640302146, 243196.44162111220, -5.8339865111793188));
        m_extractionArea.push_back(DPoint3d::From(194598.22362684523, 243191.95373863229, -5.8671826917925500));
        m_extractionArea.push_back(DPoint3d::From(194609.38532911500, 243161.44529671190, -5.8653291942937358));
        m_extractionArea.push_back(DPoint3d::From(194673.99318080919, 243145.32555555910, -6.4341180118572083));
        m_extractionArea.push_back(DPoint3d::From(194642.39430096268, 243262.27610941126, -6.5085839870498603));
        m_extractionArea.push_back(DPoint3d::From(194588.10575891405, 243347.54587653195, -6.0620844558270619));
        m_extractionArea.push_back(DPoint3d::From(194589.00309018759, 243358.31679448404, -6.2144776919703872));
        m_extractionArea.push_back(DPoint3d::From(194534.61905953390, 243398.93278513860, -5.8858629763999488));
*/

        //Small nagpur
/*
        m_extractionArea.push_back(DPoint3d::From(298487.32047431514, 2331963.9819948073, 239.52607983687085));
        m_extractionArea.push_back(DPoint3d::From(298520.54010478914, 2332041.1416489473, 239.85987082856627));
        m_extractionArea.push_back(DPoint3d::From(298556.39099312248, 2332025.8413337343, 238.89883979226761));
        m_extractionArea.push_back(DPoint3d::From(298525.30925965920, 2331972.7015292840, 238.74582114152690));
        m_extractionArea.push_back(DPoint3d::From(298503.60138231976, 2331953.4527456285, 238.32288249351404));
        m_extractionArea.push_back(DPoint3d::From(298496.20096959040, 2331955.0979408128, 240.18742513359939));
        m_extractionArea.push_back(DPoint3d::From(298487.32047431514, 2331963.9819948073, 239.52607983687085));
*/
        }

    return SUCCESS;
    }

StatusInt ScalableMeshGroundExtractor::_SetDataSourceDir(const BeFileName& dataSourceDir) 
    {
    if (!BeFileName::DoesPathExist(dataSourceDir.c_str()))
        {
        BeFileNameStatus dirCreateStatus = BeFileName::CreateNewDirectory(dataSourceDir.c_str());
        assert(BeFileNameStatus::Success == dirCreateStatus);
        }            

    m_dataSourceDir = dataSourceDir;    
    return SUCCESS;
    }  

StatusInt ScalableMeshGroundExtractor::_SetReprojectElevation(bool doReproject)
    {
    m_reprojectElevation = doReproject;
    return SUCCESS;
    }

StatusInt ScalableMeshGroundExtractor::_SetGroundPreviewer(IScalableMeshGroundPreviewerPtr& groundPreviewer)
    {
    m_groundPreviewer = groundPreviewer;
    return SUCCESS;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
