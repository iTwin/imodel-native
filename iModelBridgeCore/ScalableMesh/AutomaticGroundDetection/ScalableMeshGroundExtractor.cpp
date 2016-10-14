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

USING_NAMESPACE_GROUND_DETECTION

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor - Begin
+----------------------------------------------------------------------------*/
IScalableMeshGroundExtractorPtr IScalableMeshGroundExtractor::Create(IScalableMeshPtr& scalableMesh)
    {
    IScalableMeshGroundExtractorPtr groundExtractor(ScalableMeshGroundExtractor::Create(scalableMesh).get());
    return groundExtractor;
    }

StatusInt IScalableMeshGroundExtractor::ExtractAndEmbed()
    {
    return _ExtractAndEmbed();
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
            m_xyzFile = fopen("D:\\MyDoc\\RM - SM - Sprint 15\\AutoGroundDetect\\ATP\\detectGround.xyz", "w+");
            }

        ~ScalableMeshPointsAccumulator()
            {
            fclose(m_xyzFile);
            }

    };



ScalableMeshGroundExtractorPtr ScalableMeshGroundExtractor::Create(IScalableMeshPtr& scalableMesh)
    {
    return new ScalableMeshGroundExtractor(scalableMesh);
    }

ScalableMeshGroundExtractor::ScalableMeshGroundExtractor(IScalableMeshPtr& scalableMesh)
    {
    m_scalableMesh = scalableMesh;
    }

ScalableMeshGroundExtractor::~ScalableMeshGroundExtractor()
    {
    }

StatusInt ScalableMeshGroundExtractor::_ExtractAndEmbed()
    {    
    IGroundDetectionServices* serviceP(GroundDetectionManager::GetServices());

    bvector<DPoint3d> seedpoints;    
    GroundDetectionParametersPtr params(GroundDetectionParameters::Create());    
    params->SetLargestStructureSize(60);

    ScalableMeshPointsProviderCreatorPtr smPtsProviderCreator(ScalableMeshPointsProviderCreator::Create(m_scalableMesh));    

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
    
    return status;        
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
