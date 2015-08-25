/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSourceCreator.h $
|    $RCSfile: ScalableMeshSourceCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2015/07/15 11:02:24 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

        StatusInt                           SyncWithSources(const IDTMFile::File::Ptr&              filePtr);

        StatusInt                           ImportSourcesTo(Import::Sink*                           sinkP);

        template <typename PointIndex>
        StatusInt                           RemoveSourcesFrom(PointIndex& pointIndex, list<IDTMFile::Extent3d64f> listRemoveExtent) const;
        virtual IDTMFile::File::Ptr                 SetupFileForCreation() override;
        StatusInt                           SaveSources();
        StatusInt                           SaveSources(IDTMFile::File&                         file);
        StatusInt                           UpdateLastModified();
        void                                ResetLastModified();

        virtual bool                        IsFileDirty() override;
        virtual StatusInt                   Save(IDTMFile::File::Ptr& filePtr) override;
        virtual StatusInt                   Load(IDTMFile::File::Ptr& filePtr) override;

        void InitSources();

    public:
        explicit                            Impl(const WChar*                          scmFileName);
        explicit                            Impl(const IScalableMeshPtr&                        iDTMFilePtr);

        ~Impl();

        StatusInt                           LoadSources(const IDTMFile::File&                   file);
        virtual StatusInt                           CreateScalableMesh() override;
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE