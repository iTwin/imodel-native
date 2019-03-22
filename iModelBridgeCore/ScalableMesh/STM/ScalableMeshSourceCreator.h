/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreator.h $
|    $RCSfile: ScalableMeshSourceCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 11:02:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshSourceCreator.h>

#include "ScalableMeshCreator.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


#ifndef SM_ONE_SPLIT_THRESHOLD
    #define SM_ONE_SPLIT_THRESHOLD 10000
#endif 

#ifndef SM_BIG_SPLIT_THRESHOLD
    #define SM_BIG_SPLIT_THRESHOLD 5000000
#endif


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Elenie.Godzaridis   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceCreator::Impl : public IScalableMeshCreator::Impl, public EditListener
    {
    private:

        friend struct                       IScalableMeshSourceCreator;
        IDTMSourceCollection                m_sources;
        DRange2d                            m_extent;
        bvector<DPoint3d>                   m_filterPolygon;
        ScalableMeshCreationMethod          m_sourceCreationMethod = SCM_CREATION_METHOD_ONE_SPLIT;
        ScalableMeshCreationCompleteness    m_sourceCreationCompleteness = SCM_CREATION_COMPLETENESS_FULL;
        uint32_t                            m_splitThreshold = SM_ONE_SPLIT_THRESHOLD;

        Time                                m_lastSourcesModificationTime;
        Time                                m_lastSourcesModificationCheckTime;
        bool                                m_sourcesDirty;

        DocumentEnv                         m_sourceEnv;

        virtual void                        _NotifyOfPublicEdit() override;
        virtual void                        _NotifyOfLastEditUpdate(Time                                    updatedLastEditTime) override;

        static DocumentEnv                  CreateSourceEnvFrom(const WChar*                          filePath);

        int                                 ImportClipMaskSource(const IDTMSource&                   dataSource,
                                                                 const ClipShapeStoragePtr&              clipShapeStoragePtr) const;

        int                                 TraverseSource(SourcesImporter&                        importer,
                                                           IDTMSource&                             dataSource,
                                                           const HFCPtr<HVEClipShape>&             clipShapePtr,
                                                           const GeoCoords::GCS&                   targetGCS,
                                                           const Import::ScalableMeshData&         targetScalableMeshData) const;

        int                                 TraverseSourceCollection(SourcesImporter&                        importer,
                                                                     IDTMSourceCollection&                   sources,
                                                                     const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                     const GeoCoords::GCS&                   targetGCS,
                                                                     const Import::ScalableMeshData&         targetScalableMeshData);
        //NEEDS_WORK_SM: refactor this by adding "source type" filter to TraverseSourceCollection
        int                                 TraverseSourceCollectionEditsOnly(SourcesImporter&                        importer,
                                                                     IDTMSourceCollection&                   sources,
                                                                     const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                     const GeoCoords::GCS&                   targetGCS,
                                                                     const Import::ScalableMeshData&         targetScalableMeshData);
        int                                 TraverseSourceCollectionRasters(bvector<IDTMSource*>&                                  filteredSources,
                                                                              IDTMSourceCollection&                   sources,
                                                                              const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                              const GeoCoords::GCS&                   targetGCS,
                                                                              const Import::ScalableMeshData&         targetScalableMeshData);


        StatusInt                           ApplyEditsFromSources(HFCPtr<MeshIndexType>& pIndex);
        
        StatusInt                           GetLocalSourceTextureProvider(ITextureProviderPtr& textureProviderPtr, bvector<IDTMSource*>& filteredSources);
        

        StatusInt                           ImportRasterSourcesTo(HFCPtr<MeshIndexType>& pIndex);

        StatusInt                           ImportSourcesTo(Import::Sink*                           sinkP);

        template <typename PointIndex>
        StatusInt                           RemoveSourcesFrom(PointIndex& pointIndex, list<DRange3d> listRemoveExtent) const;


        StatusInt                           UpdateLastModified();
        void                                ResetLastModified();

        virtual bool                        IsFileDirty() override;

        virtual StatusInt                   Save() override;
        virtual StatusInt                   Load() override;
        void                                SetupFileForCreation(bool doPartialUpdate);// override;
        StatusInt                           SyncWithSources(bool restrictLevelForPropagation);
        StatusInt                           SaveSources(SMSQLiteFilePtr& smSQLiteFile);


        void InitSources();

    protected:

        StatusInt                           GetRasterSources(bvector<IDTMSource*>& filteredSources);

        StatusInt                           GetTextureProvider(ITextureProviderPtr& textureProviderPtr);

        StatusInt                           GetTextureProvider(ITextureProviderPtr& textureProviderPtr, IScalableMeshPtr& smPtr, HFCPtr<MeshIndexType>& dataIndexPtr);

        virtual void ConfigureMesherFilter(ISMPointIndexFilter<PointType, PointIndexExtentType>*& pFilter, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher2d, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher3d) override;

    public:
        explicit                            Impl(const WChar*                          scmFileName);
        explicit                            Impl(const IScalableMeshPtr&                        iDTMFilePtr);

        virtual ~Impl();

        StatusInt                           LoadSources(SMSQLiteFilePtr& smSQLiteFile);

        virtual StatusInt                   CreateScalableMesh(bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate) override;

        virtual void                        SetSplitThreshold(uint32_t splitThreshold);        
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE