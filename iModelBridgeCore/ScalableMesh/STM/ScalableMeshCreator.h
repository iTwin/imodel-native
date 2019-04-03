/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshCreator.h $
|    $RCSfile: ScalableMeshCreator.h,v $
|   $Revision: 1.45 $
|       $Date: 2011/12/21 17:04:24 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
#if _WIN32
#include <windows.h> //for showing info.
#endif

#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreatorWorker.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include "ScalableMeshProgress.h"

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
//#include <ImagePP/all/h/ISMStore.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>

#include "ScalableMeshEditListener.h"
#include "ScalableMeshStorage.h"
#include "Stores/SMSQLiteStore.h"

//#include <HGF3DExtent.h>

/*__PUBLISH_SECTION_END__*/

// NTERAY: Using namespace directive should never happen to be in headers (but inside other namespaces in some cases).
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*__PUBLISH_SECTION_START__*/
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

extern ISMPointIndexFilter<DPoint3d, Extent3dType>* s_filter;

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Sink;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourcesImporter;
class ClipShapeStorage;

typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;
typedef RefCountedPtr<IStorage>             IStoragePtr;
typedef RefCountedPtr<ClipShapeStorage>     ClipShapeStoragePtr;

typedef DPoint3d                          PointType;
//typedef ISMStore::Extent3d64f                         PointIndexExtentType;
typedef DRange3d PointIndexExtentType;
typedef SMMeshIndex <PointType, PointIndexExtentType> MeshIndexType;


BENTLEY_SM_EXPORT void RegisterDelayedImporters();
inline bool fileExist(const WChar* fileName)
    {
#if _WIN32
    ifstream file(fileName, ios::in | ios::binary);
#else
    Utf8String fileUtf8(fileName);
    ifstream file(fileUtf8.c_str(), ios::in | ios::binary);
#endif
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
        friend struct                       IScalableMeshSourceCreatorWorker::Impl;
        friend struct                       IScalableMeshSourceCreator::Impl;
        friend struct                       IScalableMeshNodeCreator::Impl;
        friend struct                       SaveAsNodeCreator;


        GeoCoords::GCS                      m_gcs;
        ScalableMeshCompressionType         m_compressionType;
        WString                             m_scmFileName;    
        IScalableMeshPtr                    m_scmPtr;  


        SMSQLiteFilePtr                     m_smSQLitePtr;
        bool                                m_isDgnDb;

        Time                                m_lastSyncTime;

        bool                                m_gcsDirty;

        const size_t                        m_workingLayer;

        bool m_isShareable;

        //CREATOR2
        static DataSourceManager            s_dataSourceManager;

        BENTLEY_SM_EXPORT StatusInt                           GetStreamedTextureProvider(ITextureProviderPtr& textureStreamProviderPtr, const WString& url);

        BENTLEY_SM_EXPORT StatusInt                           GetStreamedTextureProvider(ITextureProviderPtr& textureStreamProviderPtr, IScalableMeshPtr& smPtr, HFCPtr<MeshIndexType>& dataIndexPtr, const WString& url);

        BENTLEY_SM_EXPORT StatusInt                           CreateDataIndex(HFCPtr<MeshIndexType>& pDataIndex,
                                                                              bool                   needBalancing = false, 
                                                                              uint32_t               splitThreshold = 10000);



        
        template <typename PointType, typename PointIndex, typename LinearIndex>
        static StatusInt                    AddPointOverviewOfLinears      (PointIndex&                             pointIndex,
                                                                            LinearIndex&                            linearIndex);

        BENTLEY_SM_EXPORT bool                                FileExist                      () const;


        //ISMStore::File::Ptr                 GetFile(const ISMStore::AccessMode&             accessMode);
        BENTLEY_SM_EXPORT virtual StatusInt                           Save();
        BENTLEY_SM_EXPORT virtual StatusInt                           Load();
        BENTLEY_SM_EXPORT void                                        SetupFileForCreation();
        BENTLEY_SM_EXPORT SMSQLiteFilePtr                             GetFile(bool fileExists);

        BENTLEY_SM_EXPORT virtual bool                                IsFileDirty();

        IScalableMeshProgressPtr m_progress;

    protected:
        

        std::atomic<bool>        m_isCanceled;

        template <typename PointIndex>
          StatusInt                    Filter(PointIndex&                             pointIndex,
                                            int                                     levelToFilter = -1);

        template <typename PointIndex>
         StatusInt                    BalanceDown(PointIndex& pointIndex,
                                                  size_t      depthBeforePartialUpdate, 
                                                  bool        splitNode = false,
                                                  bool        propagateDownImmediately = true);

        template <typename PointIndex>
          StatusInt                    Mesh(PointIndex&                             pointIndex);

        template <typename PointIndex>
          StatusInt                    Stitch(PointIndex&                             pointIndex,
                                                                   int                                     levelToStitch = -1,
                                                                   bool                                    do2_5dStitchFirst = false);


          HFCPtr<MeshIndexType>                   m_pDataIndex;

          BENTLEY_SM_EXPORT ScalableMeshDb* GetDatabaseFile();

    protected:

        BENTLEY_SM_EXPORT virtual void ConfigureMesherFilter(ISMPointIndexFilter<PointType, PointIndexExtentType>*& pFilter, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher2d, ISMPointIndexMesher<PointType, PointIndexExtentType>*& pMesher3d);

        BENTLEY_SM_EXPORT void SetThreadingOptions(bool useThreadsInMeshing, bool useThreadsInStitching, bool useThreadsInFiltering);
          
    public :  

        BENTLEY_SM_EXPORT explicit                            Impl                           (const WChar*                          scmFileName);
        BENTLEY_SM_EXPORT explicit                            Impl                           (const IScalableMeshPtr&                        iDTMFilePtr);

        BENTLEY_SM_EXPORT                                     ~Impl                          ();

        BENTLEY_SM_EXPORT const GeoCoords::GCS&               GetGCS                         () const;


        BENTLEY_SM_EXPORT StatusInt                           LoadGCS();
        BENTLEY_SM_EXPORT StatusInt                           SaveGCS();

        BENTLEY_SM_EXPORT StatusInt                           LoadFromFile                   ();
        StatusInt                           SaveToFile                     ();  

        BENTLEY_SM_EXPORT virtual StatusInt                           CreateScalableMesh                    (bool isSingleFile, bool restrictLevelForPropagation, bool doPartialUpdate);

        StatusInt  SetTextureMosaic(MOSAIC_TYPE* mosaicP);
        StatusInt  SetTextureProvider(ITextureProviderPtr provider);
        StatusInt  SetTextureStreamFromUrl(WString url);

        StatusInt                           Filter                         ();        

        BENTLEY_SM_EXPORT IScalableMeshProgressPtr            GetProgress();

        bool                               IsCanceled();

        void                               Cancel();

        BENTLEY_SM_EXPORT bool IsShareable();

        BENTLEY_SM_EXPORT virtual void SetShareable(bool isShareable);        

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
StatusInt IScalableMeshCreator::Impl::BalanceDown  (PointIndex& pointIndex, size_t depthBeforePartialUpdate, bool splitNode, bool propagateDownImmediately)
    {
    pointIndex.BalanceDown(depthBeforePartialUpdate, false, splitNode, propagateDownImmediately);
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
    pointIndex.Stitch(levelToStitch, do2_5dStitchFirst);
    return BSISUCCESS;
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
