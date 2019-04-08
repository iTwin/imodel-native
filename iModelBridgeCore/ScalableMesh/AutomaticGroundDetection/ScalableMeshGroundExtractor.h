/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/ScalableMeshGroundExtractor.h $
|    $RCSfile: ScalableMesh.h,v $
|   $Revision: 1.54 $
|       $Date: 2012/01/06 16:30:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshGroundExtractor.h>
#include <ScalableMesh\IScalableMesh.h>
#include <TerrainModel\AutomaticGroundDetection\IGroundDetectionServices.h>

#include "../STM/ImagePPHeaders.h"
#include "../STM/ScalableMeshCreator.h"

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshGroundExtractor;

typedef RefCountedPtr<ScalableMeshGroundExtractor> ScalableMeshGroundExtractorPtr;

/*----------------------------------------------------------------------------+
|Class ScalableMeshBase
+----------------------------------------------------------------------------*/
struct ScalableMeshGroundExtractor : public RefCounted<IScalableMeshGroundExtractor>
    {
    private: 

        IScalableMeshPtr  m_scalableMesh;
        bvector<DPoint3d> m_extractionArea;       
        WString           m_smTerrainPath;
        IScalableMeshGroundPreviewerPtr  m_groundPreviewer;
        double            m_smGcsRatioToMeter; 
        ScalableMeshProgress m_createProgress;
        BaseGCSPtr           m_destinationGcs;
		bool                 m_limitTextureResolution;
        bool                 m_reprojectElevation;
        BeFileName           m_dataSourceDir;        
        
        
        void AddXYZFilePointsAsSeedPoints(TerrainModel::GroundDetection::GroundDetectionParametersPtr& params, const BeFileName& coverageTempDataFolder);

        double ComputeTextureResolution();

		SMStatus CreateSmTerrain(const BeFileName& coverageTempDataFolder);

    protected:                   

        virtual SMStatus                   _ExtractAndEmbed(const BeFileName& coverageTempDataFolder) override;

        virtual StatusInt                   _SetDestinationGcs(GeoCoordinates::BaseGCSPtr& destinationGcs) override;
                
        virtual StatusInt                   _SetExtractionArea(const bvector<DPoint3d>& area) override;

        virtual StatusInt                   _SetDataSourceDir(const BeFileName& dataSourceDir) override;

        virtual StatusInt                   _SetReprojectElevation(bool doReproject) override;

		virtual StatusInt                   _SetLimitTextureResolution(bool limitTextureResolution) override;

        virtual StatusInt                   _SetGroundPreviewer(IScalableMeshGroundPreviewerPtr& groundPreviewer) override;

        explicit                            ScalableMeshGroundExtractor(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh);

        virtual                             ~ScalableMeshGroundExtractor();
    
public:

    static ScalableMeshGroundExtractorPtr Create(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh);        
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
