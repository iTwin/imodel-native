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

#include <TerrainModel\AutomaticGroundDetection\GroundDetectionMacros.h>
#include <TerrainModel\AutomaticGroundDetection\GroundDetectionManager.h>
#include <TerrainModel\AutomaticGroundDetection\IGroundDetectionServices.h>
#include <TerrainModel\AutomaticGroundDetection\IPointsAccumulator.h>

#include <ScalableMesh\IScalableMeshSourceCreator.h>
#include <ScalableMesh\IScalableMeshSources.h>

USING_NAMESPACE_GROUND_DETECTION

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

BeFileName GetTempXyzFilePath()
    {
    BeFileName tempPath;
    BeFileNameStatus status = BeFileName::BeGetTempPath(tempPath);
    assert(status == BeFileNameStatus::Success);
    tempPath.AppendToPath(L"detectGround.xyz");

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

StatusInt IScalableMeshGroundExtractor::ExtractAndEmbed()
    {
    return _ExtractAndEmbed();
    }        

StatusInt IScalableMeshGroundExtractor::SetExtractionArea(const bvector<DPoint3d>& area)
    {
    return _SetExtractionArea(area);
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

StatusInt ScalableMeshGroundExtractor::CreateSmTerrain()
    {
    StatusInt status;
            
    IScalableMeshSourceCreatorPtr terrainCreator(IScalableMeshSourceCreator::GetFor(m_smTerrainPath.c_str(), status));

    assert(status == SUCCESS);
        
    if (m_scalableMesh->GetBaseGCS().IsValid())
        status = terrainCreator->SetBaseGCS(m_scalableMesh->GetBaseGCS());

    assert(status == SUCCESS);

    BeFileName xyzFile(GetTempXyzFilePath());

    IDTMLocalFileSourcePtr groundPtsSource(IDTMLocalFileSource::Create(DTM_SOURCE_DATA_POINT, xyzFile.c_str()));
    terrainCreator->EditSources().Add(groundPtsSource);

    status = terrainCreator->Create();
    assert(status == SUCCESS);
    terrainCreator->SaveToFile();
    terrainCreator = nullptr;

    return status;
    }

#define LARGEST_STRUCTURE_SIZE_DEFAULT 60 

StatusInt ScalableMeshGroundExtractor::_ExtractAndEmbed()
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

    status = CreateSmTerrain();
    
    return status;        
    } 

StatusInt ScalableMeshGroundExtractor::_SetExtractionArea(const bvector<DPoint3d>& area) 
    {
    m_extractionArea.insert(m_extractionArea.end(), area.begin(), area.end());
    return SUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
