/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreator.h $
|    $RCSfile: ScalableMeshSourceCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 11:02:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/IScalableMeshSourceCreator.h>

#include "ScalableMeshCreator.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Elenie.Godzaridis   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshSourceCreator::Impl : public IScalableMeshCreator::Impl, public EditListener
    {
    private:

        friend struct                       IScalableMeshSourceCreator;
        IDTMSourceCollection                m_sources;
        DRange2d                           m_extent;
        bvector<DPoint3d>                 m_filterPolygon;

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

        StatusInt                           GetRasterSources(bvector<IDTMSource*>& filteredSources);

        StatusInt                           GetLocalSourceTextureProvider(ITextureProviderPtr& textureProviderPtr, bvector<IDTMSource*>& filteredSources);
        
        StatusInt                           GetTextureProvider(ITextureProviderPtr& textureProviderPtr);

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

    public:
        explicit                            Impl(const WChar*                          scmFileName);
        explicit                            Impl(const IScalableMeshPtr&                        iDTMFilePtr);

        ~Impl();

        StatusInt                           LoadSources(SMSQLiteFilePtr& smSQLiteFile);

        virtual StatusInt                   CreateScalableMesh(bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate) override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE