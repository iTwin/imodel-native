/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.h $
|    $RCSfile: ScalableMeshCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshNewFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

// NTERAY: A whole lot of dependencies for a single header... This is bad. Reduce these to strict minimum.

#include <ctime> //benchmarking
#include <windows.h> //for showing info.


#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include Imagepp header files                                          |
+----------------------------------------------------------------------*/
#include <ImagePP/all/h/HVEClipShape.h>

/*----------------------------------------------------------------------+
| Core DTM Methods                                                      |
+----------------------------------------------------------------------*/

#include "ScalableMeshCoreDefs.h"
#include "ScalableMeshCoreFns.h"

#include <ScalableMesh/GeoCoords/Reprojection.h>
#include <ImagePP/all/h/IDTMFile.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>

#include "ScalableMeshEditListener.h"
#include "ScalableMeshStorage.h"
#include "SMPointTileStore.h"

//#include <HGF3DExtent.h>

/*__PUBLISH_SECTION_END__*/

// NTERAY: Using namespace directive should never happen to be in headers (but inside other namespaces in some cases).
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_START__*/
using namespace Bentley::GeoCoordinates;


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Sink;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourcesImporter;
class ClipShapeStorage;

typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;
typedef RefCountedPtr<IStorage>             IStoragePtr;
typedef RefCountedPtr<ClipShapeStorage>     ClipShapeStoragePtr;

typedef IDTMFile::Point3d64f                          PointType;
typedef IDTMFile::Extent3d64f                         PointIndexExtentType;
typedef SMMeshIndex <PointType, PointIndexExtentType> IndexType;

void RegisterDelayedImporters();
inline bool fileExist(const WChar* fileName)
    {
    ifstream file(fileName, ios::in | ios::binary);
    return file.good() && (char_traits<char>::eof() != file.peek());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IScalableMeshCreator::Impl 
    {    
    private :
        
        friend struct                       IScalableMeshCreator;
        friend struct                       IScalableMeshSourceCreator::Impl;
        friend struct                       IScalableMeshNodeCreator::Impl;

       // IDTMSourceCollection                m_sources;
        GeoCoords::GCS                      m_gcs;
        ScalableMeshCompressionType         m_compressionType;
        WString                             m_scmFileName;    
        IScalableMeshPtr                    m_scmPtr;  
#ifdef SCALABLE_MESH_DGN
        IDgnDbScalableMeshPtr               m_dgnScalableMeshPtr;
        bool                                m_isDgnDb;
#endif
        Time                                m_lastSyncTime;
#if 0
        Time                                m_lastSourcesModificationTime;
        Time                                m_lastSourcesModificationCheckTime;
        bool                                m_sourcesDirty;
#endif
        bool                                m_gcsDirty;
       // DocumentEnv                         m_sourceEnv;
        const size_t                        m_workingLayer;

        //CREATOR2

#if 0
        virtual void                        _NotifyOfPublicEdit            () override;
        virtual void                        _NotifyOfLastEditUpdate        (Time                                    updatedLastEditTime) override;

        static DocumentEnv                  CreateSourceEnvFrom            (const WChar*                          filePath);

        int                                 ImportClipMaskSource           (const IDTMSource&                   dataSource,
                                                                            const ClipShapeStoragePtr&              clipShapeStoragePtr) const;

        int                                 TraverseSource                 (SourcesImporter&                        importer,
                                                                            IDTMSource&                             dataSource,
                                                                            const HFCPtr<HVEClipShape>&             clipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS,
                                                                            const Import::ScalableMeshData&         targetScalableMeshData) const;
        
        int                                 TraverseSourceCollection       (SourcesImporter&                        importer,
                                                                            IDTMSourceCollection&                   sources,                                              
                                                                            const HFCPtr<HVEClipShape>&             totalClipShapePtr,
                                                                            const GeoCoords::GCS&                   targetGCS,
                                                                            const Import::ScalableMeshData&         targetScalableMeshData);     
#endif

        StatusInt                           CreateDataIndex (HFCPtr<IndexType>&                                    pDataIndex, 
                                                             const IDTMFile::File::Ptr&                            filePtr, 
                                                             HPMMemoryMgrReuseAlreadyAllocatedBlocksWithAlignment& myMemMgr);

#if 0
        StatusInt                           SyncWithSources                     (const IDTMFile::File::Ptr&              filePtr);

        StatusInt                           ImportSourcesTo                (Import::Sink*                           sinkP);

        template <typename PointIndex>
        StatusInt                           RemoveSourcesFrom(PointIndex& pointIndex, list<IDTMFile::Extent3d64f> listRemoveExtent) const;
#endif
        
        template <typename PointType, typename PointIndex, typename LinearIndex>
        static StatusInt                    AddPointOverviewOfLinears      (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);

        bool                                FileExist                      () const;

        IDTMFile::File::Ptr                 GetFile                        (const IDTMFile::AccessMode&             accessMode);

        virtual bool                                IsFileDirty();

        virtual StatusInt                           Save(IDTMFile::File::Ptr& filePtr);
        virtual StatusInt                           Load(IDTMFile::File::Ptr& filePtr);

        virtual IDTMFile::File::Ptr                 SetupFileForCreation           ();
#if 0

        StatusInt                           SaveSources                    ();  
        StatusInt                           SaveSources                    (IDTMFile::File&                         file);
#endif
    protected:
        template <typename PointIndex>
          StatusInt                    Filter(PointIndex&                             pointIndex,
                                            int                                     levelToFilter = -1);

        template <typename PointIndex>
         StatusInt                    BalanceDown(PointIndex& pointIndex,
                                                 size_t      depthBeforePartialUpdate);

        template <typename PointIndex>
          StatusInt                    Mesh(PointIndex&                             pointIndex);

        template <typename PointIndex>
          StatusInt                    Stitch(PointIndex&                             pointIndex,
                                                                   int                                     levelToStitch = -1,
                                                                   bool                                    do2_5dStitchFirst = false);

          HFCPtr<IndexType>                   m_pDataIndex;
    public :  
        explicit                            Impl                           (const WChar*                          scmFileName);
        explicit                            Impl                           (const IScalableMeshPtr&                        iDTMFilePtr);

                                            ~Impl                          ();

        const GeoCoords::GCS&               GetGCS                         () const;
        StatusInt                           SaveGCS                        (IDTMFile::File&                         file);


#if 0                     
        StatusInt                           UpdateLastModified             ();
        void                                ResetLastModified              ();
#endif
        StatusInt                           LoadGCS                        (const IDTMFile::File&                   file);
        //StatusInt                           LoadSources                    (const IDTMFile::File&                   file);

        StatusInt                           LoadFromFile                   ();
        StatusInt                           SaveToFile                     ();  

        virtual StatusInt                           CreateScalableMesh                    ();  
        StatusInt                           Filter                         ();

      //  IScalableMeshNodePtr                AddChildNode (const IScalableMeshNodePtr& parentNode, 
       //                                                   StatusInt&                  status);

        // For debugging / benchmarking
        static void ShowMessageBoxWithTimes(double meshingDuration, double filteringDuration, double stitchingDuration);
    };


    /*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Filter  (PointIndex&     pointIndex,
                                        int             levelToFilter)
    {    
    pointIndex.Filter(levelToFilter);   
    return BSISUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::BalanceDown  (PointIndex& pointIndex, size_t depthBeforePartialUpdate)
    {
    pointIndex.BalanceDown(depthBeforePartialUpdate);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Mesh  (PointIndex& pointIndex)
    {
    pointIndex.Mesh();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Mathieu.St-Pierre   09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointIndex>
StatusInt IScalableMeshCreator::Impl::Stitch  (PointIndex& pointIndex, 
                                        int         levelToStitch, 
                                        bool        do2_5dStitchFirst)
    {
    //pointIndex.Stitch(levelToStitch, do2_5dStitchFirst);
    return BSISUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
