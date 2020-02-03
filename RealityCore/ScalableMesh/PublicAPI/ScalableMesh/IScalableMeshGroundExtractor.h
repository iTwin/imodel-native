/*--------------------------------------------------------------------------------------+
|
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>

#undef static_assert

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct IScalableMeshGroundExtractor;

typedef RefCountedPtr<IScalableMeshGroundExtractor> IScalableMeshGroundExtractorPtr;



/*=================================================================================**//**
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshGroundExtractor: virtual public RefCountedBase
    {
    private:        

    protected:                         
             
        //Synchonization with data sources functions
        virtual SMStatus _ExtractAndEmbed(const BeFileName& coverageTempDataFolder) = 0;        

        virtual StatusInt _SetDataSourceDir(const BeFileName& dataSourceDir) = 0;

        virtual StatusInt _SetDestinationGcs(GeoCoordinates::BaseGCSPtr& destinationGcs) = 0;

        virtual StatusInt _SetExtractionArea(const bvector<DPoint3d>& area) = 0;        

        virtual StatusInt _SetReprojectElevation(bool doReproject) = 0;

        virtual StatusInt _SetLimitTextureResolution(bool limitTextureResolution) = 0;

        virtual StatusInt _SetGroundPreviewer(IScalableMeshGroundPreviewerPtr& groundPreviewer) = 0;       
        
          
    public:
      
        BENTLEY_SM_IMPORT_EXPORT SMStatus ExtractAndEmbed(const BeFileName& coverageTempDataFolder);

        BENTLEY_SM_IMPORT_EXPORT StatusInt SetDataSourceDir(const BeFileName& dataSourceDir);

        BENTLEY_SM_IMPORT_EXPORT StatusInt SetDestinationGcs(GeoCoordinates::BaseGCSPtr& destinationGcs);
        
        BENTLEY_SM_IMPORT_EXPORT StatusInt SetExtractionArea(const bvector<DPoint3d>& area);        

        BENTLEY_SM_IMPORT_EXPORT StatusInt SetReprojectElevation(bool doReproject);

        BENTLEY_SM_IMPORT_EXPORT StatusInt SetLimitTextureResolution(bool limitTextureResolution);

        BENTLEY_SM_IMPORT_EXPORT StatusInt SetGroundPreviewer(IScalableMeshGroundPreviewerPtr& groundPreviewer);
        
        BENTLEY_SM_IMPORT_EXPORT static IScalableMeshGroundExtractorPtr Create(const WString& smTerrainPath, IScalableMeshPtr& scalableMesh);

        BENTLEY_SM_IMPORT_EXPORT static void GetTempDataLocation(BeFileName& textureSubFolderName , BeFileName& extraLinearFeatureFileName);

    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
